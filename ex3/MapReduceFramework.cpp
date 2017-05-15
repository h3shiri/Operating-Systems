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
#define KRED  "\x1B[31m"
#define KMAG  "\x1B[35m"
#define RST  "\x1B[0m"


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
 * simple init for the reduce threads.
 */
pthread_mutex_t initReduceThreadsMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Protecting the map threads counter
 */
pthread_mutex_t protectMapCounter = PTHREAD_MUTEX_INITIALIZER;

/**
 * counting the number of Map User level threads that finished.
 */
static int mapThreadCounter = 0;

/**
 * index of next <k3Base*,list<v3Base*>> to take by next execReduce thread from
 * shuffleOutput
 */
static int reduceIndex = 0;

/**
 * index in k1v1bec to take a chunk from by execMap threads
 */
static int retrivalIndex = 0;

/**
 * map of pthread_t: mapThreadData
 */
static map<pthread_t, mapThreadData> k2v2Map;

/**
 * map reduce to use by the map and reduce threads
 */
static MapReduceBase * mapReduceToUse = nullptr;


/**
 * vector contain pairs of <k1Base*,v1Base*> to deal with.
 * aka our input from the user.
 */
static IN_ITEMS_VEC k1v1Vec = vector<IN_ITEM>();

/**
 * tells if the framework is responsible to free k2v2 values
 */
static bool k2v2memoryResponse;

/**
 * map for the shuffles output
 */
static map<k2Base*, vector<v2Base*>> shuffleOutPut = map<k2Base*, vector<v2Base*>>();

/**
 * map for the execReduceThreads
 */
static map<pthread_t, std::list<pair<k3Base*,v3Base*>>> reduceOutput =
        map<pthread_t, std::list<pair<k3Base*,v3Base*>>>();

/**
 * indicates that execMap threads finished
 */
static bool execMapFinished = false;

/**
 * * number of userLevelThreads we defined.
 */
static int totalThreads = 0;

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
void sortOutputAndExit(vector<OUT_ITEM> * retVal);
bool comparator(pair<k3Base*, v3Base*> p1, pair<k3Base*, v3Base*> p2);

void printLikeAManiac(string eMsg);
void testingShuffleOutputs();

/**
 * initialize mutexes and semaphore that will be used
 */
void initMutexSemaphore()
{
	//TODO: problematic having no credit for the shuffle.
    if((sem_init(&shuffleSem, 0, 1)))
    {
        errorDetect("sem_init");
    }
    //TODO: remove testing prints
//    int val;
//    sem_getvalue(&shuffleSem,&val);
//    cout << "init" << val << std::endl;
}


void mapAndShuffle(int multiThreadLevel)
{
    //initialize threads
    lock(&k2v2MapMutex);
    for(int i=0; i < multiThreadLevel;++i)
    {
        pthread_t thread;
        if(pthread_create(&thread, nullptr, &execMapRutine, nullptr) != 0)
        {
            errorDetect("pthread_create");
        }
        k2v2Map[thread] = mapThreadData();
    }
    //shuffle
    pthread_t shuffle;
    if(pthread_create(&shuffle,NULL, shuffleRutine,NULL) != 0)
    {
        errorDetect("pthread_create");
    }
    unlock(&k2v2MapMutex);
    for(map<pthread_t, mapThreadData>::iterator it = k2v2Map.begin();
        it!=k2v2Map.end(); ++it)
    {
        if (pthread_join((*it).first, NULL) != 0)
        {
            errorDetect("pthread_join");
        }
    }
    if(sem_post(&shuffleSem) !=0)
    {
        errorDetect("sem_post");
    }
	if(pthread_join(shuffle, NULL) != 0)
	{
		errorDetect("pthread_join");
	}
}

/**
 *
 * @param dummyArg - only to use properly as start routine
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
//            cout << KRED << pthread_self() << endl;
			mySIndex = retrivalIndex;
			chunkSize = min(CHUNK_SIZE, elementsNum  - retrivalIndex);
			retrivalIndex  = retrivalIndex + chunkSize;
			unlock(&retrivalIndexMutex);
			myEIndex = mySIndex + chunkSize;
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
    lock(&protectMapCounter);
    mapThreadCounter++;
    unlock(&protectMapCounter);
    sem_post(&shuffleSem);
}

void initExecReduceThreads(int multiThreadLevel)
{
    lock(&initReduceThreadsMutex);
	for(int i=0; i < multiThreadLevel;i++)
	{
		pthread_t thread;
		if(pthread_create(&thread,NULL, execReduce,NULL) != 0)
		{
			errorDetect("pthread_create");
		}
		reduceOutput[thread] = list<OUT_ITEM>();
	}
    unlock(&initReduceThreadsMutex);
    for(map<pthread_t, std::list<OUT_ITEM>>::iterator it = reduceOutput.begin();
	it!=reduceOutput.end();++it)
	{
		if (pthread_join((*it).first, NULL) != 0)
		{
			errorDetect("pthread_join");
		}
	}

}

// TODO: solve Problematic logic causes infinite loop.
void* execReduce(void* dummyArg)
{
    lock(&initReduceThreadsMutex);
    unlock(&initReduceThreadsMutex);
	int elementsNum = shuffleOutPut.size();
	int mySIndex = 0;
	int myEIndex = 0;
	int chunkStep;

	while(true)
	{
		lock(&reduceIndexMutex);
		if(reduceIndex < elementsNum)
		{
			mySIndex = reduceIndex;
			chunkStep = min(CHUNK_SIZE, elementsNum  - reduceIndex);
			myEIndex  = reduceIndex + chunkStep; // aka where to start next iteration.
			reduceIndex = myEIndex;
			unlock(&reduceIndexMutex);
		}
		// In case there are no more elements to iterate over.
		else
		{
			unlock(&reduceIndexMutex);
			break;
		}
		// should potentially start iterating from a certain point.
        map<k2Base*, vector<v2Base*>>::iterator iter;
        iter = std::next(shuffleOutPut.begin(), mySIndex);
        /* iterating over the val2 elements in the vector */
		for (int i = mySIndex; i < myEIndex; ++i)
		{
			const k2Base * k2  = (*iter).first;
			//Copying the vector should protect us from some issues.
			const vector<v2Base*> v{ begin((*iter).second), end((*iter).second) };
			mapReduceToUse->Reduce(k2, v);
			//TODO: remove deletion to a more secure location it might causes some errors.
            //due to the fact several threads are trying to mess with the same data structure on the same time.
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
//			(*iter).second.clear();
			++iter;
		}
	}
}




void* shuffleRutine(void* dummyArg)
{
	//TODO: treat semaphore?
    int length = 0;
//	int treatedPairs = 0;
	bool choosenk2p;
	while(true) {
        //TODO: clean this function
        /* this is the outer layer iterator */
        for (std::map<pthread_t, mapThreadData>::iterator it1=k2v2Map.begin();
			 it1 != k2v2Map.end(); ++it1)
		{
			lock(&(*it1).second._itemsMutex);
			vector <k2v2pair>* itemsp = &(*it1).second._k2v2items;
			length = itemsp->size();
            while(length)
            {
				k2Base *k = (*itemsp).at(length-1).first;
				v2Base *v = (*itemsp).at(length-1).second;
				for (auto& it : shuffleOutPut) {
					choosenk2p = false;
					if (equal(*(it.first), (*k))) {
						shuffleOutPut[(it.first)].push_back(v);
						choosenk2p = true;
						if (k2v2memoryResponse) {
							//to check if done properly
							delete k;
							k = NULL;
						}
						//to write the position and use erase
						itemsp->erase(itemsp->begin()+ length-1);
//						treatedPairs++;
                        length--;
						break;
					}
				}
				if (!choosenk2p) {
					shuffleOutPut[(*itemsp)[length-1].first] = vector<v2Base *>();
					(shuffleOutPut[(*itemsp)[length-1].first]).push_back((*itemsp)[length-1].second);
					itemsp->erase(itemsp->begin() + length-1);
//                    treatedPairs++;
                    length--;
				}

            }
			unlock(&(*it1).second._itemsMutex);
		}
        //WHY: treatedPairs == 0 ??
        if(execMapFinished) {
            //TEST: test extra iteration
            break;
        }
        lock(&protectMapCounter);
        if (mapThreadCounter >= totalThreads){
            execMapFinished = true;
        }
        unlock(&protectMapCounter);
        sem_wait(&shuffleSem);
    }
//    testingShuffleOutputs();
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
	totalThreads = multiThreadLevel;
    //TODO: treat case of vetorSize = 0, i.e no items to map
	mapReduceToUse = &mapReduce;
    k1v1Vec = itemsVec;
    k2v2memoryResponse = autoDeleteV2K2;
	initMutexSemaphore();
    mapAndShuffle(multiThreadLevel);
	initExecReduceThreads(multiThreadLevel);
	vector<OUT_ITEM> * retVal = new vector<OUT_ITEM>();
	sortOutputAndExit(retVal);
    return *retVal;
}

void Emit2 (k2Base* k2, v2Base* v2)
{
	pthread_t id = pthread_self();
	lock(&k2v2Map.at(id)._itemsMutex);
	k2v2Map.at(id)._k2v2items.push_back(std::make_pair(k2,v2));
	unlock(&k2v2Map.at(id)._itemsMutex);
//    TEST1: turning off the sem_shuffle.
	if(sem_post(&shuffleSem) != 0)
    {
        errorDetect("sem_post");
    }
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

// map<pthread_t, list<pair<k3Base*,v3Base*>>> reduceOutput;
void sortOutputAndExit(vector<OUT_ITEM> * retVal)
{
	vector<OUT_ITEM> temp = vector<OUT_ITEM>();;
	for (map<pthread_t, std::list<pair<k3Base*,v3Base*>>>::iterator
                 it = reduceOutput.begin(); it != reduceOutput.end(); ++it)
	{
		temp = { begin(it->second), end(it->second)};
        //TODO: remove print later.
        if(temp.size() == 0){
            continue;
        }
		retVal->insert(retVal->end(), temp.begin(), temp.end());
	}
	sort(retVal->begin(), retVal->end(), comparator);
}


bool comparator(pair<k3Base*, v3Base*> p1, pair<k3Base*, v3Base*> p2)
{
    k3Base * k1 = p1.first;
    k3Base * k2 = p2.first;
	return ((*k1) < (*k2));
}

//TODO: clean garbage

void printLikeAManiac(string eMsg){
    cout << KRED << eMsg << endl;
}

void printSamVal(){
	int * val = nullptr;
	sem_getvalue(&shuffleSem, val);
	cout << KMAG << val << endl;
}

void testingShuffleOutputs() {
    printLikeAManiac("endOfshuffle");
    for (map<k2Base*, vector<v2Base*>>::iterator iterLOL = shuffleOutPut.begin(); iterLOL != shuffleOutPut.end(); ++iterLOL)
    {
        cout << RST << iterLOL->second.size() << endl;
    }
}