// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Client.h>
#include <carla/client/World.h>

static void SetTimeout(carla::client::Client &client, double seconds) {
  client.SetTimeout(TimeDurationFromSeconds(seconds));
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host"), arg("port"), arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("get_world", &cc::Client::GetWorld)
    .def("reload_world", &cc::Client::ReloadWorld)
    .def("load_world", &cc::Client::LoadWorld, (arg("map_name")))
    .def("start_recorder", &cc::Client::StartRecorder, (arg("name")))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", &cc::Client::ShowRecorderFileInfo, (arg("name")))
    .def("show_recorder_collisions", &cc::Client::ShowRecorderCollisions, (arg("name")), (arg("type1")), (arg("type2")))
    .def("show_recorder_actors_blocked", &cc::Client::ShowRecorderActorsBlocked, (arg("name")), (arg("min_time")), (arg("min_distance")))
    .def("replay_file", &cc::Client::ReplayFile, (arg("name"), arg("time_start"), arg("duration"), arg("follow_id")))
  ;
}
