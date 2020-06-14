#pragma once

template<class T>
class CTemplateRingBuffer
{
private:

	int m_BufferSize;
	T* m_Data;
	int m_CurrentPosition;

public:
    CTemplateRingBuffer()
    {
        this->m_Data = nullptr;
    }

	CTemplateRingBuffer(int _bufferSize)
	{
        if (this->m_Data != nullptr)
        {
            delete this->m_Data;
        }
        this->m_Data = new T[_bufferSize];
        m_BufferSize = _bufferSize;
        m_CurrentPosition = 0;
    }

    ~CTemplateRingBuffer()
	{
        if (this->m_Data != nullptr)
        {
            delete this->m_Data;
        }
        
	}

    int getBufferSize()
    {
        return this->m_BufferSize;
    }

    int getCurrentPosition()
    {
        return this->m_CurrentPosition;
    }

	T getLastValue()
	{
		if (this->m_CurrentPosition == 0)
		{
			return this->m_Data[m_BufferSize -1];
		}

		return this->m_Data[this->m_CurrentPosition - 1];
	}

	void addValue(T* _value)
	{
		this->m_Data[m_CurrentPosition] = *_value;
		this->m_CurrentPosition++;

		if (this->m_CurrentPosition == m_BufferSize)
		{
			this->m_CurrentPosition = 0;
		}
	}

	T getData(int _position)
	{
		int returnPosition = this->m_CurrentPosition - 1 - _position;

		if (returnPosition < 0 )
		{
			returnPosition += this->m_BufferSize;
		}

		return this->m_Data[returnPosition];
	}

    T operator[](int _index)
    {
        return m_Data[_index];
    }
};

