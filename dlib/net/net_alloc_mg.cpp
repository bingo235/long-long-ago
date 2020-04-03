#include "net_alloc_mg.h"

dlib::net::CNetAllocItem::CNetAllocItem( uint32_t item_size, uint32_t max_count )
{
	m_item_size = item_size;
	m_item_max_count = max_count;
	m_item_free_count = 0;
	m_item_count = 0;
}

dlib::net::CNetAllocItem::~CNetAllocItem()
{	
	LIST_MEMPOINT::iterator iter = m_list_for_delete.begin();
	for (; iter != m_list_for_delete.end(); ++iter)
	{
		delete[] *iter;
		*iter = NULL;
	}
	m_list_for_delete.clear();
	m_list_free.clear();
}

void dlib::net::CNetAllocItem::AddItem()
{
	if(m_item_count >= m_item_max_count)
	{
		NETLOG_ERROR("m_item_count >= m_item_max_count  count="<<m_item_count
			<<" max="<<m_item_max_count<<FUN_FILE_LINE);
		return;
	}

	uint32_t add_count = sc_max_new_size / m_item_size;
	add_count = add_count > m_item_max_count ? m_item_max_count : add_count;
	add_count = add_count == 0 ? 1 : add_count;

	char* add_mem = new (std::nothrow) char[m_item_size*add_count];
	if(0 == add_mem)
	{
		NETLOG_ERROR("new mem fail"<<FUN_FILE_LINE);
		return;
	}
	char* point = NULL;
	for (uint32_t i = 0; i < add_count; ++i)
	{
		point = add_mem + i*m_item_size;
		m_list_free.push_back(point);
	}
	m_list_for_delete.push_back(add_mem);
	m_item_free_count += add_count;
	m_item_count += add_count;
}

bool dlib::net::CNetAllocItem::AllocBuffer( char*& buffer )
{
	if(0 == m_item_free_count)
		AddItem();
	if(0 == m_item_free_count)
		return false;

	buffer = m_list_free.front();
	m_list_free.pop_front();
	--m_item_free_count;
	return true;
}

void dlib::net::CNetAllocItem::FreeBuffer( char*& buffer )
{
	m_list_free.push_back(buffer);
	++m_item_free_count;
}

dlib::net::CNetAllocMg::CNetAllocMg()
{

}

dlib::net::CNetAllocMg::~CNetAllocMg()
{
	MAP_ALLOCITEM::iterator iter = m_map_alloc.begin();
	for (; iter != m_map_alloc.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}
	m_map_alloc.clear();
}

bool dlib::net::CNetAllocMg::AddAlloc( uint32_t alloc_size, uint32_t max_count )
{
	MAP_ALLOCITEM::iterator iter = m_map_alloc.find(alloc_size);
	if(iter != m_map_alloc.end())
		return true;

	CNetAllocItem* item = new(std::nothrow) CNetAllocItem(alloc_size, max_count);
	if(0 == item)
	{
		NETLOG_ERROR("new mem fail"<<FUN_FILE_LINE);
		return false;
	}

	m_map_alloc[alloc_size] = item;
	return true;
}

bool dlib::net::CNetAllocMg::AllocBuffer( char*& buffer, uint32_t size )
{
	MAP_ALLOCITEM::iterator iter = m_map_alloc.find(size);
	if(iter == m_map_alloc.end())
	{
		NETLOG_ERROR("AllocBuffer fail. size="<<size<<FUN_FILE_LINE);
		return false;
	}

	return iter->second->AllocBuffer(buffer);
}

bool dlib::net::CNetAllocMg::FreeBuffer( char*& buffer, uint32_t size )
{
	MAP_ALLOCITEM::iterator iter = m_map_alloc.find(size);
	if(iter == m_map_alloc.end())
	{
		NETLOG_ERROR("FreeBuffer fail. size="<<size<<FUN_FILE_LINE);
		return false;
	}

	iter->second->FreeBuffer(buffer);
    return true;
}

void dlib::net::CNetAllocMg::AutoTest()
{
	CNetAllocMg mg;
	mg.AddAlloc(1024*10, 1000);
	mg.AddAlloc(1024*20, 1000);
	char* buff = NULL;
	for (uint32_t i = 0; i < 10; ++i)
	{
		mg.AllocBuffer(buff, 1024*10);
		strcpy(buff, "1024*10\n");
		printf(buff);
		mg.FreeBuffer(buff, 1024*10);

		mg.AllocBuffer(buff, 1024*20
			
			);
		strcpy(buff, "1024*20\n");
		printf(buff);
		mg.FreeBuffer(buff, 1024*20);
	}
}


