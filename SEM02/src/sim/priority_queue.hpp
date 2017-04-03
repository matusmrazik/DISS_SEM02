#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <functional>
#include <queue>
#include <mutex>

template <typename T, typename Container = std::vector<T>, typename Comp = std::less<T> >
class priority_queue
{
public:
	using value_type = typename Container::value_type;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	using size_type = typename Container::size_type;

public:
	priority_queue()
	{
	}

	~priority_queue()
	{
	}

	void push(const value_type &val)
	{
		std::lock_guard<std::mutex> lk(_mutex);
		_queue.push(val);
	}

	void push(value_type &&val)
	{
		std::lock_guard<std::mutex> lk(_mutex);
		_queue.push(val);
	}

	const_reference top()
	{
		std::lock_guard<std::mutex> lk(_mutex);
		return _queue.top();
	}

	void pop()
	{
		std::lock_guard<std::mutex> lk(_mutex);
		_queue.pop();
	}

	value_type top_pop()
	{
		std::lock_guard<std::mutex> lk(_mutex);
		value_type ret = _queue.top();
		_queue.pop();
		return ret;
	}

	size_type size() const
	{
		std::lock_guard<std::mutex> lk(_mutex);
		return _queue.size();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lk(_mutex);
		return _queue.empty();
	}

private:
	std::priority_queue<T, Container, Comp> _queue;
	mutable std::mutex _mutex;
};

#endif // PRIORITY_QUEUE_HPP
