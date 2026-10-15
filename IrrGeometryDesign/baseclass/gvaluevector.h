/****************************************************************************
** GValueVector.h   1.0   edited Aug 19 06:00 $
**
** Definition of GValueVector based template class
**
** Created : def
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
**********************************************************************/

#ifndef GVALUEVECTOR_H
#define GVALUEVECTOR_H

typedef unsigned int uint;

template<class type>
class GValueVector	//指针线性列表
{
private:
	type *m_pdata;
	uint m_size;
	uint m_count;
private:
	void sift( uint k, uint m, int (* compare)(type *d1, type *d2) );	//堆排序筛子函数
public:
	GValueVector<type>();				//构造空表
	GValueVector<type>( uint size );	//预先分配空间size
	~GValueVector<type>();

	inline uint size() { return m_size; }
	inline uint count() { return m_count; }
	inline bool isEmpty() { return m_count == 0; }
	inline bool isNull() { return m_pdata == NULL; }

	bool prepend( type data );	//前插
	bool append( type data );	//后插
	bool insert( uint index, type data );	//插入
	uint insertByOrder( type data, int (* compare)(const type &d1, const type &d2) ); //插入并有序

	bool removeFirst();			//删除第一元素
	bool removeLast();			//删除尾元素
	bool remove( uint index );	//删除index上的元素
	void clear();				//清空

	inline type at( uint index, bool *ok = NULL );	//返回index上的元素
	inline bool set( uint index, type data );	//设置index上的元
	inline type & operator []( uint index ) { return m_pdata[index]; }	

	void resize( uint size );	//重新分配空间
	void truncate() { resize(count()); }

	int find( type d, uint index = 0 );	//线性查找
	int find( type d, int (* compare)(const type &d1, const type &d2), uint index = 0 );

	int bsearch( type d, int (* compare)(const type &d1, const type &d2) );	//二分查找

	void heapsort( int (* compare)(type *d1, type *d2) );	//堆排序
};

template<class type>
GValueVector<type>::GValueVector<type>()
{
	m_pdata = NULL;
	m_size = 0;
	m_count = 0;
}

template<class type>
GValueVector<type>::GValueVector<type>( uint size )
{
	if( size > 0 ) m_pdata = new type [size];
	else m_pdata = NULL;
	m_size = size;
	m_count = 0;
}

template<class type>
GValueVector<type>::~GValueVector<type>()
{
	if( m_pdata != NULL ) 
	{
		delete []m_pdata;
	}
}

template<class type>
void GValueVector<type>::clear()
{
	m_count = 0;
}

template<class type>
void GValueVector<type>::resize( uint size )
{
	uint i;
	type *pdata;
	if( size > m_size )			//扩张空间
	{
		pdata = new type [size];
		for( i=0; i<m_count; i++ ) pdata[i] = m_pdata[i];
		delete []m_pdata;
		m_pdata = pdata;
	}
	else if( size < m_size )	//缩小空间
	{
		if( size == 0 ) 
		{
			clear();
			delete []m_pdata;
			m_pdata = NULL;
			m_count = 0;
		}
		else
		{
			pdata = new type [size];
			for( i=0; i<m_count&&i<size; i++ ) pdata[i] = m_pdata[i];
			m_count = i;
			delete []m_pdata;
			m_pdata = pdata;
		}
	}
	m_size = size;
}

template<class type>
bool GValueVector<type>::append( type data )
{
	if( m_count == m_size )		//需要扩张空间
	{
		type *pdata;
		int size = m_size+m_size/2;
		size = (size/8+1)*8;	//对齐8边距减少内存碎片
		pdata = new type [size];
		if( pdata == NULL ) return false;
		m_size = size;
		for( uint i=0; i<m_count; i++ ) pdata[i] = m_pdata[i];
		if( m_pdata != NULL ) delete []m_pdata;
		m_pdata = pdata;
	}
	m_pdata[m_count++] = data;
	return true;
}

template<class type>
bool GValueVector<type>::prepend( type data )
{
	uint i;
	if( m_count == m_size )		//需要扩张空间
	{
		type *pdata;
		int size = m_size+m_size/2;
		size = (size/8+1)*8;	//对齐8边距减少内存碎片
		pdata = new type [size];
		if( pdata == NULL ) return false;
		m_size = size;
		for( i=0; i<m_count; i++ ) pdata[i+1] = m_pdata[i];
		if( m_pdata != NULL ) delete []m_pdata;
		m_pdata = pdata;
	}
	else 
	{
		for( i=m_count; i>0; i-- ) m_pdata[i] = m_pdata[i-1];
	}
	m_pdata[0] = data;
	m_count++;
	return true;
}

template<class type>
bool GValueVector<type>::insert( uint index, type data )
{
	if( index > m_count ) return false;

	uint i;
	if( m_count == m_size )		//需要扩张空间
	{
		type *pdata;
		int size = m_size+m_size/2;
		size = (size/8+1)*8;	//对齐8边距减少内存碎片
		pdata = new type [size];
		if( pdata == NULL ) return false;
		m_size = size;
		for( i=0; i<index; i++ ) pdata[i] = m_pdata[i];
		for( i=index; i<m_count; i++ ) pdata[i+1] = m_pdata[i];
		if( m_pdata != NULL ) delete []m_pdata;
		m_pdata = pdata;
	}
	else
	{
		for( i=m_count; i>index; i-- ) m_pdata[i] = m_pdata[i-1];
	}
	m_pdata[index] = data;
	m_count++;
	return true;
}

template<class type>
uint GValueVector<type>::insertByOrder( type data, int (* compare)(const type &d1, const type &d2) )
{
	int i, index;
	if( m_count == m_size )		//需要扩张空间
	{
		type *pdata;
		m_size += m_size/2;
		m_size = (m_size/8+1)*8;	//对齐8边距减少内存碎片
		pdata = new type [m_size];
		for( i=m_count-1; i>=0; i-- )
		{
			if( compare( data, m_pdata[i] ) >= 0 ) break;
			pdata[i+1] = m_pdata[i];
		}
		index = i+1;
		pdata[index] = data;
		for( ; i>=0; i-- ) pdata[i] = m_pdata[i];
		if( m_pdata != NULL ) delete []m_pdata;
		m_pdata = pdata;
	}
	else
	{
		for( i=m_count-1; i>=0; i-- )
		{
			if( compare( data, m_pdata[i] ) >= 0 ) break;
			m_pdata[i+1] = m_pdata[i];
		}
		index = i+1;
		m_pdata[index] = data;
	}
	m_count++;
	return index;
}

template<class type>
bool GValueVector<type>::removeFirst()
{
	if( m_count == 0 ) return false;

	for( uint i=1; i<m_count; i++ ) m_pdata[i-1] = m_pdata[i];	
	m_count--;
	return true;
}

template<class type>
bool GValueVector<type>::removeLast()
{
	if( m_count == 0 ) return false;

	m_count--;
	delete m_pdata[m_count];
	return true;
}

template<class type>
bool GValueVector<type>::remove( uint index )
{
	if( index >= m_count ) return false;

	m_count--;
	for( uint i=index; i<m_count; i++ ) m_pdata[i] = m_pdata[i+1];
	return true;
}

template<class type>
type GValueVector<type>::at( uint index, bool *ok )
{
	if( ok != NULL ) 
	{
		if( 0 <= index && index < m_count ) *ok = true;
		else *ok = false;
	}
	return m_pdata[index];
}

template<class type>
bool GValueVector<type>::set( uint index, type data )
{
	if( index >= m_count ) return false;
	m_pdata[index] = data;
	return true;
}

template<class type>
int GValueVector<type>::find( type d, uint index )
{
	for( uint i=index; i<m_count; i++ )
	{
		if( m_pdata[i] == d ) return i;
	}
	return -1;
}

template<class type>
int GValueVector<type>::find( type d, int (* compare)(const type &d1, const type &d2), uint index )
{
	for( uint i=index; i<m_count; i++ )
	{
		if( compare(m_pdata[i], d) == 0 ) return i;
	}
	return -1;
}

template<class type>
int GValueVector<type>::bsearch( type d, int (* compare)(const type &d1, const type &d2) )
{
	uint i, first, last;

	first = 0;	
	last = m_count-1;
	while( first <= last )
	{
		i = (first+last)/2;
		int r = compare(m_pdata[i], d);
		if( r > 0 ) last = i-1;
		else if( r == 0 ) return i;
		else first = i+1;
	}
	
	return -1;
}

template<class type>
void GValueVector<type>::sift( uint k, uint m, int (* compare)(type *d1, type *d2) )
{
    uint i,j;
    type rp;

    i = k;	j = 2*i+1;	rp = m_pdata[k];
    while( j <= m ) 
	{
	    if( j < m && compare(&m_pdata[j], &m_pdata[j+1]) < 0 ) j++;
	    if( compare(&m_pdata[j], &rp) < 0 ) break;
	    else 
		{ 
			m_pdata[i] = m_pdata[j]; 
			i = j; 
			j = 2*i+1; 
		}
	}
    m_pdata[i] = rp;
}	

template<class type>
void GValueVector<type>::heapsort( int (* compare)(type *d1, type *d2) )
{
	int i;
	type rp;

    for( i=m_count/2-1; i>=0; i--) sift( i, m_count-1, compare );
    for( i=m_count-1; i>=1; i--)
	{
	    rp = m_pdata[0];
	    m_pdata[0] = m_pdata[i];
	    m_pdata[i] = rp;
	    sift( 0, i-1, compare );
	}
}

#endif
