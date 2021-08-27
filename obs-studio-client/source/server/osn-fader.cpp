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

#include "osn-fader.hpp"
#include "error.hpp"
#include "obs.h"
#include "osn-source.hpp"
#include "shared-server.hpp"
#include "utility-server.hpp"

obs::Fader::Manager& obs::Fader::Manager::GetInstance()
{
	static obs::Fader::Manager _inst;
	return _inst;
}

void obs::Fader::ClearFaders()
{
    Manager::GetInstance().for_each([](obs_fader_t* fader)
    { 
        obs_fader_destroy(fader);
    });

    Manager::GetInstance().clear();
}

uint64_t obs::Fader::Create(int32_t fader_type)
{
	obs_fader_type type = (obs_fader_type)fader_type;

	obs_fader_t* fader = obs_fader_create(type);
	if (!fader) {
		blog(LOG_ERROR, "Failed to create Fader.");
	}

	auto uid = Manager::GetInstance().allocate(fader);
	if (uid == std::numeric_limits<utility_server::unique_id::id_t>::max()) {
		obs_fader_destroy(fader);
		blog(LOG_ERROR, "Failed to allocate unique id for Fader.");
	}

	return uid;
}

void obs::Fader::Destroy(uint64_t uid)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	obs_fader_destroy(fader);
	Manager::GetInstance().free(uid);
}

float_t obs::Fader::GetDeziBel(uint64_t uid)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	return obs_fader_get_db(fader);
}

float_t obs::Fader::SetDeziBel(uint64_t uid, float_t db)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	obs_fader_set_db(fader, db);
	return obs_fader_get_db(fader);
}

float_t obs::Fader::GetDeflection(uint64_t uid)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	return obs_fader_get_deflection(fader);
}

float_t obs::Fader::SetDeflection(uint64_t uid, float_t deflection)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	obs_fader_set_deflection(fader, deflection);
	return obs_fader_get_deflection(fader);
}

float_t obs::Fader::GetMultiplier(uint64_t uid)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	return obs_fader_get_mul(fader);
}

float_t obs::Fader::SetMultiplier(uint64_t uid, float_t mul)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	obs_fader_set_mul(fader, mul);

	return obs_fader_get_mul(fader);
}

void obs::Fader::Attach(uint64_t uid_fader, uint64_t uid_source)
{
	auto fader = Manager::GetInstance().find(uid_fader);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	auto source = obs::Source::Manager::GetInstance().find(uid_source);
	if (!source) {
		blog(LOG_ERROR, "Invalid Source Reference.");
	}

	if (!obs_fader_attach_source(fader, source)) {
		blog(LOG_ERROR, "Error attaching source.");
	}
}

void obs::Fader::Detach(uint64_t uid)
{
	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		blog(LOG_ERROR, "Invalid Fader Reference.");
	}

	obs_fader_detach_source(fader);
}

void obs::Fader::AddCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	//!FIXME!
}

void obs::Fader::RemoveCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	//!FIXME!
}
