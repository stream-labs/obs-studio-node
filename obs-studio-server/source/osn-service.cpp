/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "osn-service.hpp"
#include <error.hpp>
#include "shared.hpp"

void osn::Service::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Service");
	cls->register_function(
	    std::make_shared<ipc::function>("GetTypes", std::vector<ipc::type>{}, GetTypes));
	cls->register_function(std::make_shared<ipc::function>(
	    "Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "Create",
	    std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String, ipc::type::String},
	    Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "CreatePrivate", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>(
	    "CreatePrivate",
	    std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String},
	    CreatePrivate));
	cls->register_function(
	    std::make_shared<ipc::function>("GetName", std::vector<ipc::type>{ipc::type::UInt64}, GetName));
	cls->register_function(
	    std::make_shared<ipc::function>("GetProperties", std::vector<ipc::type>{ipc::type::UInt64}, GetProperties));
	cls->register_function(
	    std::make_shared<ipc::function>("Update", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, Update));
	cls->register_function(
	    std::make_shared<ipc::function>("GetSettings", std::vector<ipc::type>{ipc::type::UInt64}, GetSettings));
	cls->register_function(
	    std::make_shared<ipc::function>("GetURL", std::vector<ipc::type>{ipc::type::UInt64}, GetURL));
	cls->register_function(
	    std::make_shared<ipc::function>("GetKey", std::vector<ipc::type>{ipc::type::UInt64}, GetKey));
	cls->register_function(
	    std::make_shared<ipc::function>("GetUsername", std::vector<ipc::type>{ipc::type::UInt64}, GetUsername));
	cls->register_function(
	    std::make_shared<ipc::function>("GetPassword", std::vector<ipc::type>{ipc::type::UInt64}, GetPassword));

    srv.register_collection(cls);
}

void osn::Service::GetTypes(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

    size_t index = 0;
    const char* type = "";
    while (obs_enum_service_types(index++, &type)) {
        if (type)
            rval.push_back(ipc::value(type));
    }
	AUTO_DEBUG;
}

void osn::Service::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string serviceId, name;
	obs_data_t *settings = nullptr, *hotkeys = nullptr;

	switch (args.size()) {
	case 4:
		hotkeys = obs_data_create_from_json(args[3].value_str.c_str());
	case 3:
		settings = obs_data_create_from_json(args[2].value_str.c_str());
	case 2:
		name      = args[1].value_str;
		serviceId = args[0].value_str;
		break;
	}

	obs_service_t* service = obs_service_create(serviceId.c_str(), name.c_str(), settings, hotkeys);
	if (!service) {
        if (settings)
            obs_data_release(settings);
        if (hotkeys)
            obs_data_release(hotkeys);
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Failed to create service.");
	}

	uint64_t uid = osn::Service::Manager::GetInstance().allocate(service);
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
}

void osn::Service::CreatePrivate(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string serviceId, name;
	obs_data_t *settings = nullptr;

	switch (args.size()) {
	case 3:
		settings = obs_data_create_from_json(args[2].value_str.c_str());
	case 2:
		name      = args[1].value_str;
		serviceId = args[0].value_str;
		break;
	}

	obs_service_t* service = obs_service_create_private(serviceId.c_str(), name.c_str(), settings);
	if (!service) {
        if (settings)
            obs_data_release(settings);
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Failed to create service.");
	}

	uint64_t uid = osn::Service::Manager::GetInstance().allocate(service);
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
}

void osn::Service::GetName(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

    const char* name = obs_service_get_name(service);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(name ? name : ""));
}

void osn::Service::GetProperties(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

    const char* serviceId = obs_service_get_id(service);
    if (!serviceId) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service id is not valid.");
    }

	obs_properties_t* prp = obs_get_service_properties(serviceId);
	obs_data* settings = obs_service_get_settings(service);

    bool update = false;
	utility::ProcessProperties(prp, settings, update, rval);

	obs_properties_destroy(prp);

	obs_data_release(settings);
	AUTO_DEBUG;
}

void osn::Service::Update(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

	obs_data_t* settings = obs_data_create_from_json(args[1].value_str.c_str());
	obs_service_update(service, settings);
	obs_data_release(settings);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Service::GetSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

	obs_data_t* settings = obs_service_get_settings(service);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_data_get_full_json(settings)));
	obs_data_release(settings);
	AUTO_DEBUG;
}
void osn::Service::GetURL(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

    const char* url = obs_service_get_url(service);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(url ? url : ""));
}

void osn::Service::GetKey(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

    const char* key = obs_service_get_key(service);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(key ? key : ""));
}

void osn::Service::GetUsername(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

    const char* username = obs_service_get_username(service);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(username ? username : ""));
}
void osn::Service::GetPassword(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_service_t* service = osn::Service::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
    }

    const char* password = obs_service_get_password(service);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(password ? password : ""));
}

osn::Service::Manager& osn::Service::Manager::GetInstance()
{
	static osn::Service::Manager _inst;
	return _inst;
}
