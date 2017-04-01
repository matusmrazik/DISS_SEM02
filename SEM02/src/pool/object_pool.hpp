#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <cstddef>
#include <cstdlib>
#include <utility>
#include <forward_list>
#include <type_traits>

template <typename T, typename... List>
struct is_from : std::true_type {};

template <typename T, typename Head, typename... Rest>
struct is_from<T, Head, Rest...> : std::conditional<std::is_same<T, Head>::value, std::true_type, is_from<T, Rest...>>::type {};

template <typename T>
struct is_from<T> : std::false_type {};

template <typename T>
constexpr inline const T &max(const T &p)
{
	return p;
}

template <typename T>
constexpr inline const T &max(const T &p1, const T &p2)
{
	return p1 > p2 ? p1 : p2;
}

template <typename T, typename... Args>
constexpr inline const T &max(const T &p1, const T &p2, Args&&... args)
{
	return max(p1, max(p2, args...));
}


template <size_t ElemSize>
class pool_base
{
public:
	static constexpr size_t ElementSize = ElemSize;
	static constexpr size_t ElementOffset = sizeof(void*);
	static constexpr size_t ChunkSize = ElementSize + ElementOffset;

	const size_t ChunksInBlock;
	const size_t BlockSize;

public:
	explicit inline pool_base(const size_t chunks_in_block)
		: ChunksInBlock(chunks_in_block), BlockSize(ChunksInBlock * ChunkSize),
		  _first(0)
	{
	}

	~pool_base()
	{
		_free_memory();
	}

	void *alloc()
	{
		if (!_first) _add_block(); // no more free chunks, allocate new block
		if (_next(_first) == 0) // one chunk left
		{
			char * const ret = static_cast<char*>(_first);
			_first = 0;
			return ret + ElementOffset;
		}
		char * const ret = static_cast<char*>(_first); // more than one chunk, safe to just take the first one
		_first = _next(_first);
		return ret + ElementOffset;
	}

	void free(void * const chunk)
	{
		// TODO check if chunk was allocated by this pool
		void * const chunk_ptr = static_cast<char*>(chunk) - ElementOffset;
		if (!_first) // no free chunks, this chunk becomes new first
		{
			_first = chunk_ptr;
			_next(_first) = 0;
			return;
		}
		_next(chunk_ptr) = _first; // put this chunk before the first one
		_first = chunk_ptr;
	}

private:
	inline void *&_next(void * const ptr)
	{
		return *(static_cast<void**>(ptr));
	}

	void _add_block() // add block to the end of free chunks
	{
		void *block = std::malloc(BlockSize);
		_blocks.push_front(block);
		char *old = static_cast<char*>(block) + (ChunksInBlock - 1) * ChunkSize;
		_first = block;
		_next(old) = 0;
		for (char *iter = old - ChunkSize; iter != block; old = iter, iter -= ChunkSize)
			_next(iter) = old;
		_next(block) = old;
	}

	void _free_memory()
	{
		for (auto item : _blocks)
			std::free(item);
		_blocks.clear();
		_first = 0;
	}

private:
	void *_first;
	std::forward_list<void*> _blocks;
};

template <typename T>
class object_pool : protected pool_base<sizeof(T)>
{
public:
	using element_type = T;
	using pointer = element_type*;
	using base_type = pool_base<sizeof(element_type)>;

public:
	explicit inline object_pool(const size_t chunks_in_block = 32)
		: base_type(chunks_in_block)
	{
	}

	~object_pool()
	{
	}

	inline base_type &base()
	{
		return *this;
	}

	template <typename... Args>
	pointer construct(Args&&... args)
	{
		pointer ret = static_cast<pointer>(base().alloc());
		if (!ret) return ret;

		new (ret) element_type(std::forward<Args>(args)...);
		return ret;
	}

	void destroy(pointer const ptr)
	{
		ptr->~element_type();
		base().free(ptr);
	}
};

template <typename... Types>
class multi_object_pool : protected pool_base<max(sizeof(Types)...)>
{
public:
	using base_type = pool_base<max(sizeof(Types)...)>;

	template <size_t N>
	using type = typename std::tuple_element<N, std::tuple<Types...>>::type;

	static constexpr size_t NTypes = sizeof...(Types);

public:
	explicit inline multi_object_pool(const size_t chunks_in_block = 32)
		: base_type(chunks_in_block)
	{
	}

	~multi_object_pool()
	{
	}

	inline base_type &base()
	{
		return *this;
	}

	template <typename T, typename... Args>
	T *construct(Args&&... args)
	{
		static_assert(is_from<T, Types...>::value, "type is not registered in pool's type list!");

		T *ret = static_cast<T*>(base().alloc());
		if (!ret) return ret;

		new (ret) T(std::forward<Args>(args)...);
		return ret;
	}

	template <typename T>
	void destroy(T * const ptr)
	{
		static_assert(is_from<T, Types...>::value, "type is not registered in pool's type list!");

		ptr->~T();
		base().free(ptr);
	}
};

#endif // OBJECT_POOL_HPP
