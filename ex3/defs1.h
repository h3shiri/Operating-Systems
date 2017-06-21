//
// Created by hadasba on 5/7/17.
//

#ifndef EX3_DEFS_H
#define EX3_DEFS_H
#include <map>
#include <list>
#include <utility>
#include <pthread.h>
#include "MapReduceFramework.h"
#include "MapReduceClient.h"


typedef std::pair<k2Base*,v2Base*> k2v2pair;

/**
 * Struct which associate to pthread and contains
 * a list of interItems and a mutex that protects it
 */
struct mapThreadData{
    std::vector<k2v2pair> _k2v2items;
    pthread_mutex_t _itemsMutex;
    mapThreadData()
    {
        _k2v2items = std::vector<k2v2pair>();
        _itemsMutex = PTHREAD_MUTEX_INITIALIZER;
    }
};

/**
 * map: pthread_t : list(<k2*,v2*>)
 */
typedef std::map<pthread_t, mapThreadData> K2v2MAP;




	//destructor code
	// for(auto &it:foo_list) delete it; foo_list.clear();
	// if (pthread_mutex_destroy(&reduceIndexLock != 0)
	// {
	// 	errorDetect("pthread_mutex_destroy");
	// }



#endif //EX3_DEFS_H
