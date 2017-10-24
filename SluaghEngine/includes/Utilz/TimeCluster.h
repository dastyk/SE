#ifndef SE_UTILZ_TIME_CLUSTER_H_
#define SE_UTILZ_TIME_CLUSTER_H_
#include <Utilz\CompileTimeStringHasher.h>
#include <unordered_map>

namespace SE
{
	namespace Utilz
	{
		typedef std::unordered_map<Utilz::IDHash, float, Utilz::IDHash::Hasher> TimeMap;


		class TimeCluster
		{
		
		public:
			/**
			*
			* @brief Start the timing
			**/
			virtual void Start(const Utilz::IDHash& id) = 0;

			/**
			*
			* @brief Stop the timing
			**/
			virtual void Stop(const Utilz::IDHash& id) = 0;

			/**
			*
			* @brief Get the times
			**/
			virtual void GetMap(TimeMap& map) = 0;		

			virtual ~TimeCluster() {};

		protected:
			TimeCluster() {};
		};
	}
}


#endif //SE_UTILZ_TIME_CLUSTER_H_