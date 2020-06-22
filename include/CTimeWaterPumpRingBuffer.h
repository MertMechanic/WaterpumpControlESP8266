#pragma once


class CTimeWaterPump;

class CTimeWaterPumpRingBuffer
{
	public:
		CTimeWaterPumpRingBuffer();
		// CTimeWaterPumpRingBuffer(int _bufferSize);

		~CTimeWaterPumpRingBuffer();
		int getBufferSize();
		int getCurrentPosition();
		CTimeWaterPump* getLastValue();
		void addValue(CTimeWaterPump* _value);
		CTimeWaterPump* getData(int _position);
		CTimeWaterPump* operator[](int _index);
        void setBufferSize(int _size);

	private:

		int m_BufferSize;
		CTimeWaterPump* m_pDataArray;
		int m_CurrentPosition;



};

