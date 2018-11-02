#pragma once
#include "osn-source.hpp"

namespace osn
{
	class Scene : Source
	{
		public:
		static void Register(ipc::server&);

		static void
		            Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void CreatePrivate(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
		static void
		    FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		static void
		    Release(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void
		    Remove(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		static void
		    AsSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void
		    Duplicate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		static void
		            AddSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void FindItemByName(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
		static void FindItemByItemId(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
		static void
		    MoveItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void
		    GetItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void
		            GetItems(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetItemsInRange(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);

		// Signals?
		static void
		            Connect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void Disconnect(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
	};
} // namespace osn
