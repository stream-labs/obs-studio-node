// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "osn-module.hpp"
#include "error.hpp"
#include "shared.hpp"

void osn::Module::Reigster(ipc::server&)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Module");

	cls->register_function(
	    std::make_shared<ipc::function>("Open", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, Open));
	cls->register_function(
	    std::make_shared<ipc::function>("Initialize", std::vector<ipc::type>{ipc::type::UInt64}, Initialize));
	cls->register_function(
	    std::make_shared<ipc::function>("GetName", std::vector<ipc::type>{ipc::type::UInt64}, GetName));
	cls->register_function(
	    std::make_shared<ipc::function>("GetFileName", std::vector<ipc::type>{ipc::type::UInt64}, GetFileName));
	cls->register_function(
	    std::make_shared<ipc::function>("GetAuthor", std::vector<ipc::type>{ipc::type::UInt64}, GetAuthor));
	cls->register_function(
	    std::make_shared<ipc::function>("GetDescription", std::vector<ipc::type>{ipc::type::UInt64}, GetDescription));
	cls->register_function(
	    std::make_shared<ipc::function>("GetBinaryPath", std::vector<ipc::type>{ipc::type::UInt64}, GetBinaryPath));
	cls->register_function(
	    std::make_shared<ipc::function>("GetDataPath", std::vector<ipc::type>{ipc::type::UInt64}, GetDataPath));
	cls->register_function(
	    std::make_shared<ipc::function>("GetDataPath", std::vector<ipc::type>{ipc::type::UInt64}, GetDataPath));
}

void osn::Module::Open(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
{
	obs_module_t*     module;

	const std::string bin_path  = args[0].value_str.c_str();
	const std::string data_path = args[1].value_str.c_str();

	int64_t result = obs_open_module(&module, bin_path.c_str(), data_path.c_str());

	if (result == MODULE_SUCCESS) {
		uint64_t uid = osn::Module::Manager::GetInstance().allocate(module);

		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		rval.push_back(ipc::value(uid));
	} else {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create module."));
	}

	AUTO_DEBUG;
}

void osn::Module::Initialize(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}
	
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_init_module(module)));
	AUTO_DEBUG;
}

void osn::Module::GetName(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_name(module)));
	AUTO_DEBUG;
}

void osn::Module::GetFileName(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_file_name(module)));
	AUTO_DEBUG;
}

void osn::Module::GetAuthor(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_author(module)));
	AUTO_DEBUG;
}

void osn::Module::GetDescription(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_description(module)));
	AUTO_DEBUG;
}

void osn::Module::GetBinaryPath(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_binary_path(module)));
	AUTO_DEBUG;
}

void osn::Module::GetDataPath(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	obs_module_t* module = osn::Module::Manager::GetInstance().find(args[0].value_union.ui64);

	if (!module) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Module reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_get_module_data_path(module)));
	AUTO_DEBUG;
}

void osn::Module::GetFilePath(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{

}

void osn::Module::GetConfigFilePath(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{

}

osn::Module::Manager& osn::Module::Manager::GetInstance()
{
	static osn::Module::Manager _inst;
	return _inst;
}