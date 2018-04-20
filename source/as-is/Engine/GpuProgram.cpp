#include "GpuProgram.h"
#include <assert.h>
namespace HW
{
	NameGenerator *GpuProgram::m_NameGenerator = new NameGenerator("GpuProgram");

	GpuProgram::GpuProgram():m_RenderSystem(0)
	{
		m_valid = false;
		m_Name = m_NameGenerator->Generate();

	}

	GpuProgram::GpuProgram(const string &name):m_RenderSystem(0),m_Name(name)
	{
		m_valid = false;
	}


	GpuProgram::~GpuProgram()
	{
		m_valid = false;
		for (std::map<string,ProgramData>::iterator itr = m_ProgramData.begin();itr != m_ProgramData.end();itr++)
		{
			if(itr->second.type == SDT_CONSTANT)
				::operator delete(itr->second.data);
		}
	}



}
