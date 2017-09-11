#ifndef SE_UTILZ_GUID_H_
#define SE_UTILZ_GUID_H_
#include <stdint.h>
namespace SE
{
	namespace Utilz
	{
		class GUID
		{
		public:
			struct Compare
			{
				bool operator() (const GUID& lhs, const GUID& rhs) const
				{
					return lhs.id < rhs.id;
				}
			};


			GUID(uint64_t id) : id(id) {}
			GUID(const GUID& other) : id(other.id) {}
			GUID(const GUID&& other) : id(other.id) {}
			GUID& operator=(const GUID& other) { this->id = other.id; }
		private:
			uint64_t id;

			
		};


	}
}

#endif //SE_UTILZ_GUID_H_