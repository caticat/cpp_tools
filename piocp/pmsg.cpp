#include "pmsg.h"

PMsg::PMsg() : m_msg(""),
	m_pos(m_headLen),
	m_msgLen(m_headLen),
	m_proto(0)
{
	Write((uint32)0); // 将协议头清零
	//*(uint16*)(m_msg.c_str()) = (uint16)0;
	//*((uint16*)(m_msg.c_str())+1) = (uint16)0;
}

PMsg::PMsg(const char* msg,int16 msgLen) : m_msg(""),
	m_pos(m_headLen),
	m_msgLen(m_headLen),
	m_proto(0)
{
	_WriteData(msg,msgLen);
}

void PMsg::SetProto(uint16 proto)
{
	m_proto = proto;
	*((uint16*)(m_msg.c_str())+1) = m_proto;
}

uint16 PMsg::GetProto() const
{
	return m_proto;
}

uint16 PMsg::GetPos() const
{
	return m_pos;
}

uint16 PMsg::GetDataLen() const
{
	return *(uint16*)(m_msg.c_str());
}

void PMsg::ResetPos()
{
	m_pos = m_headLen;
}

void PMsg::Reset()
{
	m_msg = "";
	m_pos = m_headLen;
	m_msgLen = m_headLen;
	m_proto = 0;
	Write((uint32)0);
}

void PMsg::_WriteData(const void* pData,uint16 dataLen)
{
	m_msg.insert(m_msg.length(),(char*)pData,dataLen);
	*(uint16*)(m_msg.c_str()) = (uint16)m_msg.length()-m_headLen; // 更新长度
}
