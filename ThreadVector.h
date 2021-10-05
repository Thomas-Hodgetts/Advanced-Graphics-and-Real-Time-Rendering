#pragma once
#include <thread>
#include <vector>
#include <mutex>

template<typename T>
class ThreadVector
{
public:
	ThreadVector() { m_Vector = std::vector<std::shared_ptr<T>>(); };
	ThreadVector(const ThreadVector& obj) { m_Vector = obj.m_Vector; }
	~ThreadVector() {};

	void push_back(T var)
	{
		std::lock_guard<std::mutex> lg(m);
		m_Vector.push_back(std::make_shared<T>(var));
	};

	std::shared_ptr<T> Grab(int Pos)
	{
		std::lock_guard<std::mutex> lg(m);
		if (!m_Vector.empty() && Pos >! m_Vector.size())
		{
			std::shared_ptr<T> var(m_Vector[Pos]);
			return var;
		}
		if (Pos > m_Vector.size())
		{
			throw std::runtime_error("Postion out of memory range");
		}
		else
		{
			throw std::runtime_error("ThreadVector is Empty");
		}
	}

	std::shared_ptr<T> pop_back()
	{
		std::lock_guard<std::mutex> lg(m);
		if (!m_Vector.empty())
		{
			std::shared_ptr<T> back(m_Vector.back());
			m_Vector.pop_back();
			return back;
		}
		else
		{
			throw std::runtime_error("ThreadStack is Empty");
		}
	}

	T& at(int Pos)
	{
		std::lock_guard<std::mutex> lg(m);
		if (!m_Vector.empty() && Pos < m_Vector.size())
		{
			return *m_Vector.at(Pos);
		}
		else
		{
			throw std::runtime_error("ThreadStack is Empty");
		}
	}

	T& front()
	{
		std::lock_guard<std::mutex> lg(m);
		if (!m_Vector.empty())
		{
			return *m_Vector.front();
		}
		else
		{
			throw std::runtime_error("ThreadStack is Empty");
		}
	}

	T& back()
	{
		std::lock_guard<std::mutex> lg(m);
		if (!m_Vector.empty())
		{
			return *m_Vector.back();
		}
		else
		{
			throw std::runtime_error("ThreadStack is Empty");
		}
	}

	void reserve(int Pos)
	{
		std::lock_guard<std::mutex> lg(m);
		m_Vector.reserve(Pos);
	}

	int capacity()
	{
		return m_Vector.capacity();
	}

	void shrink_to_fit()
	{
		std::lock_guard<std::mutex> lg(m);
		m_Vector.shrink_to_fit();
	}

	void empty()
	{
		return m_Vector.empty();
	}

	void clear()
	{
		std::lock_guard<std::mutex> lg(m);
		return m_Vector.clear();
	}

	int size()
	{
		return m_Vector.size();
	}



private:

	std::vector<std::shared_ptr<T>> m_Vector;
	std::mutex m;
};