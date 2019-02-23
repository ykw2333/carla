// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Time.h"
#include "carla/rpc/RPCCallFlag.h"
#include "carla/rpc/Response.h"

#include <boost/asio/io_service.hpp>

#include <rpc/server.h>

#include <future>

namespace carla {
namespace rpc {

namespace detail {

  /// Function traits based on http://rpclib.net implementation.
  /// MIT Licensed, Copyright (c) 2015-2017, Tamás Szelei
  template <typename T>
  struct wrapper_function_traits : wrapper_function_traits<decltype(&T::operator())> {};

  template <typename C, typename R, typename... Args>
  struct wrapper_function_traits<R (C::*)(Args...)> : wrapper_function_traits<R (*)(Args...)> {};

  template <typename C, typename R, typename... Args>
  struct wrapper_function_traits<R (C::*)(Args...) const> : wrapper_function_traits<R (*)(Args...)> {};

  template <typename R, typename... Args> struct wrapper_function_traits<R (*)(Args...)> {
    using result_type = R;
    using function_type = std::function<R(RPCCallFlag, Args...)>;
    using packaged_task_type = std::packaged_task<R()>;
  };

  /// Wraps @a functor into a function type with equivalent signature. The wrap
  /// function returned, when called, posts @a functor into the io_service and
  /// waits for it to finish.
  ///
  /// This way, no matter from which thread the wrap function is called, the
  /// @a functor provided is always called from the context of the io_service.
  ///
  /// @warning The wrap function blocks until @a functor is executed by the
  /// io_service.
  template <typename F>
  inline auto WrapSyncCall(boost::asio::io_service &io, F functor) {
    using func_t = typename wrapper_function_traits<F>::function_type;
    using task_t = typename wrapper_function_traits<F>::packaged_task_type;

    return func_t([&io, functor=std::move(functor)](RPCCallFlag call_flag, auto &&... args) -> typename func_t::result_type {
      auto task = std::make_shared<task_t>([functor=std::move(functor), args...]() {
        return functor(args...);
      });
      auto result = task->get_future();
      io.post([task=std::move(task)]() mutable { (*task)(); });
      if (call_flag == RPCCallFlag::Asynchronous) {
        return ResponseError("result ignored on async call");
      }
      return result.get();
    });
  }

  template <typename F>
  inline auto WrapAsyncCall(F functor) {
    using func_t = typename wrapper_function_traits<F>::function_type;
    return func_t([functor=std::move(functor)](RPCCallFlag, auto &&... args) {
      return functor(std::forward<decltype(args)>(args)...);
    });
  }

} // namespace detail

  /// An RPC server in which functions can be bind to run synchronously or
  /// asynchronously.
  ///
  /// Use `AsyncRun` to start the worker threads, and use `SyncRunFor` to
  /// run a slice of work in the caller's thread.
  ///
  /// Functions that are bind using `BindAsync` will run asynchronously in the
  /// worker threads. Functions that are bind using `BindSync` will run within
  /// `SyncRunFor` function.
  class Server {
  public:

    template <typename ... Args>
    explicit Server(Args && ... args)
      : _server(std::forward<Args>(args) ...) {
      _server.suppress_exceptions(true);
    }

    template <typename Functor>
    void BindAsync(const std::string &name, Functor &&functor) {
      _server.bind(name, detail::WrapAsyncCall(std::forward<Functor>(functor)));
    }

    template <typename Functor>
    void BindSync(const std::string &name, Functor functor) {
      _server.bind(
          name,
          detail::WrapSyncCall(_sync_io_service, std::move(functor)));
    }

    void AsyncRun(size_t worker_threads) {
      _server.async_run(worker_threads);
    }

    void SyncRunFor(time_duration duration) {
      _sync_io_service.reset();
      _sync_io_service.run_for(duration.to_chrono());
    }

    /// @warning does not stop the game thread.
    void Stop() {
      _server.stop();
    }

  private:

    boost::asio::io_service _sync_io_service;

    ::rpc::server _server;
  };

} // namespace rpc
} // namespace carla
