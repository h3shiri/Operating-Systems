#include "MapReduceFramework.h"
#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <iterator>

#define MULTI_THREAD_LEVEL 2
#define FAILURE -1
#define SUCCESS 0
#define FOUND 1

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KBLU  "\x1B[34m"

//TODO: check its ok to print in blue:)
using namespace std;

/**
 * A class representing a directory name to search in.
 */
class k1dirName : public k1Base {
private:
	string _dirName;

public:
	k1dirName(string &dirName) : _dirName(dirName) { } 
    virtual bool operator<(const k1Base &other) const 
    {
    	k1dirName differentDir = (const k1dirName&)(other);
    	return _dirName < differentDir.getName(); 
    }

    string getName() 
    {
    	return _dirName;
    }
};

/**
 * A class which is nonessential due to the fact each file is never
 * grouped with other files, in other senarios we might change it.
 */
class k2dummy : public k2Base {
private:
	int _priority = 0; // might be used for priorities.
public:

	virtual bool operator<(const k2Base &other) const
	{
		k2dummy differentDir = (const k2dummy&) other;
		// int res = (int)(_priority < differentDir.getClassification());
		/* we actually return false to all of them to seperate the keys */
		return false; 
	}

	// A getter for the priority int.
	int getClassification(){
		return _priority;
	}
	// A setter for the priority int.
	void setClassification(int pr){
		_priority = pr;
	}
};

/**
 * A class for the file's name and the relevent boolean flag.
 * In our case this flag indicates whether the target string has been found.
 */
class v2fileName : public v2Base {
private:
	bool _relevantFlag;
	string _fileName;
public:
	v2fileName(bool _relevantFlag, string _fileName) : 
	_relevantFlag(_relevantFlag), _fileName(_fileName) {}

	/* returns the relevant flag condition */
	bool getFlag(){
		return _relevantFlag;
	}

	/* returns the relevant name */
	string getName(){
		return _fileName;
	}
};

class k3outputFile : public k3Base {
private:
	string _fileName;
public:
	k3outputFile(string fileName) : _fileName(fileName){}
	/* sorting lexicographically */
    virtual bool operator<(const k3Base &other) const
    {
    	k3outputFile differentKey = (const k3outputFile&) other;
    	return _fileName < differentKey.getName();
    }

    string getName(){
    	return _fileName;
    }
};


void getFilesFromFolder(string targetDir, vector<string>& dirFiles)
{
	const char * dirCharName = targetDir.c_str();
	DIR * dr;
	struct dirent * env;
	if ((dr = opendir(dirCharName)) != NULL)
	{
		while((env = readdir(dr)))
		{
			dirFiles.push_back(string(env->d_name));	
		}
		closedir(dr);
	}
}

class MapReduceTargetSearch : public MapReduceBase {
private:
	string _target;
public:
	MapReduceTargetSearch(string target) : _target(target) {}

	/* our mapping function takes a directory and out list of target files in the second layer */
	void Map(const k1Base *const key, const v1Base *const val) const {
		(void) val;
		k1Base * key1 = (k1Base *) key;
		k1dirName * dirPath = dynamic_cast<k1dirName *>(key1);
		vector<string> filesInDir = vector<string>();
		string dirName = dirPath->getName();
		getFilesFromFolder(dirName, filesInDir);
		for (auto& str : filesInDir)
		{
			// checking for the target substring
			if(str.find(_target) != string::npos)
			{
				k2dummy * key2 = new k2dummy();
				v2fileName * val2 = new v2fileName(FOUND, str);
				Emit2(key2, val2);
			}
		}
	}

	/* our reduce function actually just move stuff to be ready for output */
	void Reduce(const k2Base *const key, const V2_VEC &vals) const {
		//in this version we actually neglect the key
		k3outputFile * k3 = nullptr;
		(void) key; 
		for (auto * val2 : vals)
		{
			v2Base * val2_not_const = (v2Base *) val2;
			v2fileName * real_v2 = dynamic_cast<v2fileName *>(val2_not_const);
			if (real_v2->getFlag())
			{
				k3 = new k3outputFile(real_v2->getName());
				//TODO: same here who's responsibility to clean up?
				Emit3(k3, nullptr);
			}
		}
	}
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		string eMsg = "too few aruments, format: <target> <dir1, dir2..>";
		cerr << KRED << eMsg << endl;
		exit(FAILURE);
	}
	IN_ITEMS_VEC setOfKeys = vector<IN_ITEM>();
	string targetString(argv[1]);
	MapReduceTargetSearch frameworkPlayer(targetString);
	// updating all the keys for the framework.
	for (int i = 2; i < argc; ++i)
	{
		string dirName(argv[i]);
		k1dirName * tempDir = new k1dirName(dirName);
		IN_ITEM itemKey = IN_ITEM((k1Base*) tempDir, nullptr);
		setOfKeys.push_back(itemKey);
	}

	OUT_ITEMS_VEC printOutItems = 
	RunMapReduceFramework(frameworkPlayer, setOfKeys, MULTI_THREAD_LEVEL, false);
	string delim = " ";
	unsigned long wordsLeftToPrint = printOutItems.size();
	for (auto & k3v3Pair : printOutItems)
	{
		--wordsLeftToPrint;
		k3Base* basic = (k3Base*) k3v3Pair.first;
		k3outputFile* toBeEmitted = (k3outputFile*) basic;
		string outPutString = toBeEmitted->getName();
		cout << outPutString;
		if (wordsLeftToPrint > 0)
		{
			cout << delim;
		}

	}
	return SUCCESS;
}