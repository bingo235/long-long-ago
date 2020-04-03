#ifndef NET_ALLOC_MG_h_
#define NET_ALLOC_MG_h_
#include "../common/common.h"

namespace dlib
{
	namespace net
	{
		class CNetAllocItem
		{
			static const uint32_t sc_max_new_size = 1024 * 1024 * 16;
			typedef std::list<char *>		LIST_MEMPOINT;
		public:
			CNetAllocItem(uint32_t item_size, uint32_t max_count = 10000);
			~CNetAllocItem();

			bool AllocBuffer(char*& buffer);
			void FreeBuffer(char*& buffer);
			
		protected:
			void AddItem();

		private:
			uint32_t		m_item_size;
			uint32_t		m_item_max_count;
			uint32_t		m_item_free_count;
			uint32_t		m_item_count;
			LIST_MEMPOINT	m_list_free;
			LIST_MEMPOINT	m_list_for_delete;
		};

		class CNetAllocMg
		{
			typedef std::map<uint32_t, CNetAllocItem*>	MAP_ALLOCITEM;
		public:
			CNetAllocMg();
			~CNetAllocMg();

			bool AddAlloc(uint32_t alloc_size, uint32_t max_count);
			bool AllocBuffer(char*& buffer, uint32_t size);
			void FreeBuffer(char*& buffer, uint32_t size);

			static void AutoTest();
		private:
			MAP_ALLOCITEM	m_map_alloc;
		};
	}
}
#endif // NET_ALLOC_MG_h_
