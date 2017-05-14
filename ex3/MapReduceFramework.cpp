//
// Created by hadasba on 5/7/17.
//
#include "defs1.h"
#include <semaphore.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <fcntl.h>

#define CHUNK_SIZE 10
#define FAILURE -1

using namespace std;
/**
 * semaphore that will cause the shuffle to now that Emit2 instered
 * pairs of <k2Base*,v2Base*> to k2v2Map so the suffle can work
 */
sem_t shuffleSem;

/**
 * mutex that protecs the k2v2map. keep that no thread will
 * start running before map initialization
 */
pthread_mutex_t k2v2MapMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * mutex that protects reduceIndex
 */
pthread_mutex_t reduceIndexMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * mutex protects retrivalIndex
 */
pthread_mutex_t retrivalIndexMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * index of next <k3Base*,list<v3Base*>> to take by next execReduce thread from
 * shuffleOutput
 */
int reduceIndex = 0;

/**
 * index in k1v1bec to take a chunk from by execMap threads
 */
int retrivalIndex = 0;

/**
 * map of pthread_t: mapThreadData
 */
map<pthread_t, mapThreadData> k2v2Map;

/**
 * map reduce to use by the map and reduce threads
 */
MapReduceBase * mapReduceToUse = nullptr;


/**
 * vector contain pairs of <k1Base*,v1Base*> to deal with.
 * aka our input from the user.
 */
IN_ITEMS_VEC k1v1Vec;

/**
 * tells if the framework is responsible to free k2v2 values
 */
bool k2v2memoryResponse;

/**
 * map for the shuffles output
 */
map<k2Base*, vector<v2Base*>> shuffleOutPut;

/**
 * map for the execReduceThreads
 */
map<pthread_t, list<pair<k3Base*,v3Base*>>> reduceOutput;

/**
 * number of execMap threads
 */
int threadsNum = 0;

//functions declaretions:
void *  execMapRutine(void* dummyArg);
void initMutexSemaphore();
void errorDetect(string errorMsg);
void lock(pthread_mutex_t* toLock);
void unlock(pthread_mutex_t* toUnlock);
void mapAndShuffle(int multiThreadLevel);
void* shuffleRutine(void* dummyArg);
bool equal(const k2Base& fk2, const k2Base& sk2);
void* execReduce(void* dummyArg);
void initExecReduceThreads(int multiThreadLevel);
void sortOuput();


/**
 * initialize mutexes and semaphore that will be used
 */
void initMutexSemaphore(int multiThreadLevel)
{
    if((sem_init(&shuffleSem, 0, 1) ||
        sem_init(&shuffleSem, 0, multiThreadLevel) != 0 ))
    {
        errorDetect("sem_init");
    }
}




void mapAndShuffle(int multiThreadLevel)
{
    //initialize threads
    lock(&k2v2MapMutex);
    for(int i=0; i < multiThreadLevel;i++)
    {
        pthread_t thread;
        if(pthread_create(&thread, nullptr, &execMapRutine, nullptr) != 0)
        {
            errorDetect("pthread_create");
        }
        k2v2Map[thread] = mapThreadData();
    }
    unlock(&k2v2MapMutex);
	//shuffle
	pthread_t shuffle;
	if(pthread_create(&shuffle,NULL, shuffleRutine,NULL) != 0)
	{
		errorDetect("pthread_create");
	}
	if(pthread_join(shuffle, NULL) != 0)
	{
		errorDetect("pthread_join");
	}

}

/**
 *
 * @param dummyArg - only to use properly as startrutine
 * argument to pthread_create
 * @return NULL
 */
void * execMapRutine(void* dummyArg)
{
    //cause each thread to be blocked creation of threads and
    //filling k2v2map properly
    lock(&k2v2MapMutex);
    unlock(&k2v2MapMutex);
    int elementsNum = k1v1Vec.size();
	int mySIndex = 0;
	int myEIndex = 0;
	int chunkSize;
    while(true)
    {
        lock(&retrivalIndexMutex);
		if(retrivalIndex != elementsNum)
		{
			mySIndex = retrivalIndex;
			chunkSize = min(CHUNK_SIZE, elementsNum  - retrivalIndex);
			retrivalIndex  = retrivalIndex + chunkSize;
			unlock(&retrivalIndexMutex);
			myEIndex = mySIndex + chunkSize - 1;
		}
		else
		{
			unlock(&retrivalIndexMutex);
			break;
		}
		for (int k1v1Idx = mySIndex; k1v1Idx < myEIndex; k1v1Idx++)
		{
			IN_ITEM& k1v1 = k1v1Vec.at(k1v1Idx);
			mapReduceToUse->Map(k1v1.first, k1v1.second);
		}
    }

}

void initExecReduceThreads(int multiThreadLevel)
{
	for(int i=0; i < multiThreadLevel;i++)
	{
		pthread_t thread;
		if(pthread_create(&thread,NULL, execReduce,NULL) != 0)
		{
			errorDetect("pthread_create");
		}
		reduceOutput[thread] = list<OUT_ITEM>();
	}
	for(map<pthread_t, list<OUT_ITEM>>::iterator it = reduceOutput.begin();
	it!=reduceOutput.end();++it)
	{
		if (pthread_join((*it).first, NULL) != 0)
		{
			errorDetect("pthread_join");
		}
	}

}


void* execReduce(void* dummyArg)
{
	int elementsNum = shuffleOutPut.size();
	int mySIndex = 0;
	int myEIndex = 0;
	int i = 0;
	int chunkSize;
	map<k2Base*, vector<v2Base*>>::iterator iter; ;

	while(true)
	{
		lock(&reduceIndexMutex);
		if(reduceIndex != elementsNum)
		{
			mySIndex = reduceIndex;
			i = reduceIndex;
			chunkSize = min(CHUNK_SIZE, elementsNum  - reduceIndex);
			retrivalIndex  = reduceIndex + chunkSize;
			unlock(&reduceIndexMutex);
			myEIndex = mySIndex + reduceIndex - 1;
		}
		else
		{
			unlock(&reduceIndexMutex);
			break;
		}
		iter = std::next(shuffleOutPut.begin(), mySIndex);
		while( i  <= myEIndex)
		{
			k2Base * k2  = (*iter).first;
			mapReduceToUse->Reduce(k2, (*iter).second);
			if(k2v2memoryResponse)
			{
				delete k2;
				k2 = NULL;
				for(vector<v2Base*>::iterator it = (*iter).second.begin();
					it != (*iter).second.end(); ++it)
				{
					delete *it;
				}
			}
			(*iter).second.clear();
			++iter;
			++i;
		}
	}

}




void* shuffleRutine(void* dummyArg)
{
	//treat semaphore
	int length;
	int treatedPairs = 0;
	bool chosenk2p;
	int internalCount = 0;
	int pairsNum = k1v1Vec.size();
	while(true) {
		sem_wait(&shuffleSem);
		for (std::map<pthread_t, mapThreadData>::iterator it1=k2v2Map.begin();
			 it1 != k2v2Map.end(); ++it1)
		{
			lock(&(*it1).second._itemsMutex);
			vector <k2v2pair>* itemsp = &(*it1).second._k2v2items;
			length = itemsp->size();
			internalCount = 0;
			for (int i = 0; i < length; i++) {
				k2Base *k = (*itemsp).at(i).first;
				v2Base *v = (*itemsp)[i].second;
				for (auto& it : shuffleOutPut) {
					chosenk2p = false;
					if (equal(*(it.first), (*k))) {
						shuffleOutPut[(it.first)].push_back((*itemsp)[i].second);
						chosenk2p = true;
						if (k2v2memoryResponse) {
							//to check if done properly
							delete k;
							k = NULL;
						}
						//to write the position and use erase
						itemsp->erase(itemsp->begin()+ i);
						treatedPairs++;
						internalCount++;
						//down semaphore
						break;
					}
				}
				if (!chosenk2p) {
					shuffleOutPut[(*itemsp)[i].first] = vector<v2Base *>();
					(shuffleOutPut[(*itemsp)[i].first]).push_back((*itemsp)[i].second);
					itemsp->erase(itemsp->begin() + i);
					treatedPairs++;
					internalCount++;
					//itemsp->remove(items[i]);
					//down semaphore
				}


			}
			unlock(&(*it1).second._itemsMutex);
			if((itemsp->size()!=0)|| (internalCount != (length-1)))
			{
				std::cout <<"list is not empty after shuffle treat!" << endl;
			}
			for(int i = 0; i <(internalCount -1); ++i)
			{
				sem_wait(&shuffleSem);
			}
		}
		if(treatedPairs == pairsNum)
		{
			break;
		}
	}
}



/**asystem call fail
 *
 * @param functionName
 */
void errorDetect(string funcName)
{
    std::cerr << "MapReduceFramework Failure: " + std::string(funcName) +
            " failed" << std::endl;
    exit(FAILURE);
}

/**
 * unlock a given mutex
 * @param toLock - a mutex to lock
 */
void lock(pthread_mutex_t* toLock)
{
    if (pthread_mutex_lock(toLock) != 0)
    {
        errorDetect("pthread_mutex_lock");
    }
}

/**
 * unlock a given mutex
 * @param toUnlock - mutex
 */
void unlock(pthread_mutex_t* toUnlock)
{
    if (pthread_mutex_unlock(toUnlock) != 0)
    {
        errorDetect("pthread_mutex_unlock");
    }
}

OUT_ITEMS_VEC RunMapReduceFramework(MapReduceBase& mapReduce, IN_ITEMS_VEC& itemsVec,
                                    int multiThreadLevel, bool autoDeleteV2K2)
{
    //treat case of vetorSize = 0 i.e no items to map
	(*mapReduceToUse) = mapReduce;
    k1v1Vec = itemsVec;
    k2v2memoryResponse = autoDeleteV2K2;
	initMutexSemaphore();
    mapAndShuffle(multiThreadLevel);
	initExecReduceThreads(multiThreadLevel);
}

void Emit2 (k2Base* k2, v2Base* v2)
{
	pthread_t id = pthread_self();
	lock(&k2v2Map.at(id)._itemsMutex);
	k2v2Map.at(id)._k2v2items.push_back(std::make_pair(k2,v2));
	unlock(&k2v2Map.at(id)._itemsMutex);
	sem_post(&shuffleSem);

}


bool equal(const k2Base& fk1, const k2Base& fk2)
{
	if((fk1 < fk2)||(fk2 < fk1))
	{
		return false;
	}
	return true;
}

void Emit3 (k3Base* k3, v3Base* v3)
{
	reduceOutput.at(pthread_self()).push_back(make_pair(k3,v3));
}


void sortOuput()
{

}


//TODO: clean garbage



// execMap stuff

//		for (int i = mySIndex; i < myEIndex; i++)
//		{
//			//std::pair<k2Base*,std::list<k2Base*>>& k2Lv2 = shuffleOutPut.at(i);
//			k2Base* k2 = shuffleOutPut.at(i).first;
//			std::list<k2Base*>& lst = shuffleOutPut.at(i).second;
//			mapReduceToUse.Reduce(k2, lst);
//			if(k2v2memoryResponse)
//			{
//				delet k2;
//				k2 = NULL;
//				for(std::list<k2Base*>::const_iterator it = lst.begin();
//					it != lst.end(); ++it)
//				{
//					delete *it;
//				}
//				lst.clear();
//			}
//		}