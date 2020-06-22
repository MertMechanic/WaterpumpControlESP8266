#include "CTimeWaterPumpRingBuffer.h"
#include "CTimeWaterPump.h"


	CTimeWaterPumpRingBuffer::CTimeWaterPumpRingBuffer()
	{
		this->m_pDataArray = nullptr;
        this->m_BufferSize = 0;
        this->m_CurrentPosition = 0;
	}


	// CTimeWaterPumpRingBuffer::CTimeWaterPumpRingBuffer(int _bufferSize)
	// {
	// 	if (this->m_pDataArray != nullptr)
	// 	{
	// 		delete this->m_pDataArray;
	// 	}
	// 	this->m_pDataArray = new CTimeWaterPump[_bufferSize];
	// 	m_BufferSize = _bufferSize;
	// 	m_CurrentPosition = 0;
	// }


	CTimeWaterPumpRingBuffer::~CTimeWaterPumpRingBuffer()
	{
		if (this->m_pDataArray != nullptr)
		{
			delete this->m_pDataArray;
		}

	}

    void CTimeWaterPumpRingBuffer::setBufferSize(int _size)
    {
        if (this->m_pDataArray != nullptr)
		{
			delete this->m_pDataArray;
		}
		this->m_pDataArray = new CTimeWaterPump[_size];
		m_BufferSize = _size;
		m_CurrentPosition = 0;
    }

	int CTimeWaterPumpRingBuffer::getBufferSize()
	{
		return this->m_BufferSize;
	}



	int CTimeWaterPumpRingBuffer::getCurrentPosition()
	{
		return this->m_CurrentPosition;
	}


	CTimeWaterPump* CTimeWaterPumpRingBuffer::getLastValue()
	{
		if (this->m_CurrentPosition == 0)
		{
			return &this->m_pDataArray[m_BufferSize - 1];
		}

		return &this->m_pDataArray[this->m_CurrentPosition - 1];
	}

	void CTimeWaterPumpRingBuffer::addValue(CTimeWaterPump* _value)
	{
		this->m_pDataArray[m_CurrentPosition] = *_value;
		this->m_CurrentPosition++;

		if (this->m_CurrentPosition == m_BufferSize)
		{
			this->m_CurrentPosition = 0;
		}
	}


	CTimeWaterPump* CTimeWaterPumpRingBuffer::getData(int _position)
	{
		int returnPosition = this->m_CurrentPosition - 1 - _position;

		if (returnPosition < 0)
		{
			returnPosition += this->m_BufferSize;
		}

		return &this->m_pDataArray[returnPosition];
	}


	CTimeWaterPump* CTimeWaterPumpRingBuffer::operator[](int _index)
	{
		if (_index > m_BufferSize) {
			/*cout << "Index out of bounds" << endl;*/
			// return first element.
			return &m_pDataArray[0];
		}
		return &m_pDataArray[_index];
	}