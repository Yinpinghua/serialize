#ifndef _SERIALIZE_HEADER_H_
#define _SERIALIZE_HEADER_H_
#include <sstream>   //std::stringstream
#include <vector>    //std::vector
#include <list>        //std::list
#include <set>       //std::set
#include <map>     //std::map
#include <utility>    // std::pair
#include <iterator>  //std::back_inserter
#include <string.h>  //memcpy
#include <unordered_map>
#include <unordered_set>

////////////////////////////////////////////////////
// define normal template function
////////////////////////////////////////////////////

//虚函数调用
template<typename SerializableType =Serializable>
std::string serialize(SerializableType& a)
{
	return a.serialize();
}

template<typename SerializableType=Serializable>
int deserialize(std::string& str, SerializableType& a)
{
	return a.deserialize(str);
}

/////////////////////////////////////////////////
//define special template function
//Serialize for C/C++ basic type
//examples: short,int,float,long long,double
/////////////////////////////////////////////////
#define DEF_BASIC_TYPE_SERIALIZE(Type) \
 template<> \
std::string serialize(Type& b) \
{ \
        std::string ret; \
        ret.append((const char*)&b,sizeof(Type)); \
        return ret; \
}

#define DEF_BASIC_TYPE_DESERIALIZE(Type)  \
 template<> \
int deserialize(std::string& str,Type& b)\
{ \
        memcpy(&b,str.data(),sizeof(Type)); \
        return sizeof(Type); \
}

#define DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(Type) \
        DEF_BASIC_TYPE_SERIALIZE(Type) \
        DEF_BASIC_TYPE_DESERIALIZE(Type)

DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(char)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned char)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(short int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned short int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(float)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(long long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned long long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(double)

//////////////////////////////////////
//Serialize for type string
/////////////////////////////////////

// for c++ type std::string
template<>
std::string serialize(std::string& s)
{
	unsigned int len = static_cast<unsigned int>(s.size());
	std::string ret;
	ret.append(::serialize(len));
	ret.append(s.data(), len);
	return ret;
}

template<>
int deserialize(std::string& str, std::string& s)
{
	unsigned int len;
	::deserialize(str, len);
	s = str.substr(sizeof(len), len);
	return sizeof(int) + len;
}

////////////////////////////////////////////
//define input and output stream
//for serialize data struct
////////////////////////////////////////////

class out_stream
{
public:

	out_stream() : os(std::ios::binary)
	{
	}

	template<typename SerializableType>
	out_stream& operator<< (SerializableType& a)
	{
		std::string x = ::serialize(a);
		os.write(x.data(), x.size());
		return *this;
	}

	template<typename BasicType>
	out_stream& operator<< (std::vector<BasicType>& a)
	{
		unsigned int len = static_cast<unsigned int>(a.size());

		std::string x = ::serialize(len);
		os.write(x.data(), x.size());

		for (unsigned int i = 0; i < len; ++i)
		{
			std::string item = ::serialize(a[i]);
			os.write(item.data(), item.size());
		}

		return *this;
	}

	template<typename BasicType>
	out_stream& operator<< (std::list<BasicType>& a)
	{
		std::vector<BasicType> temp;
		std::copy(a.begin(), a.end(), std::back_inserter(temp));
		return this->operator<< (temp);
	}

	template<typename BasicType>
	out_stream& operator<< (std::set<BasicType>& a)
	{
		std::vector<BasicType> temp;
		std::copy(a.begin(), a.end(), std::back_inserter(temp));
		return this->operator<< (temp);
	}

	template<typename BasicType>
	out_stream& operator<< (std::unordered_set<BasicType>& a)
	{
		std::vector<BasicType> temp;
		std::copy(a.begin(), a.end(), std::back_inserter(temp));
		return this->operator<< (temp);
	}

	template<typename BasicTypeA, typename BasicTypeB>
	out_stream& operator<< (std::map<BasicTypeA, BasicTypeB>& a)
	{
		std::vector<BasicTypeA> tempKey;
		std::vector<BasicTypeB> tempVal;

		typename std::map<BasicTypeA, BasicTypeB>::const_iterator it = a.begin();
		for (;it != a.end();++it)
		{
			tempKey.push_back(it->first);
			tempVal.push_back(it->second);
		}

		this->operator<< (tempKey);
		return this->operator<< (tempVal);
	}

	template<typename BasicTypeA, typename BasicTypeB>
	out_stream& operator<< (std::unordered_map<BasicTypeA, BasicTypeB>& a)
	{
		std::vector<BasicTypeA> tempKey;
		std::vector<BasicTypeB> tempVal;

		typename std::unordered_map<BasicTypeA, BasicTypeB>::const_iterator it = a.begin();
		for (;it != a.end();++it)
		{
			tempKey.push_back(it->first);
			tempVal.push_back(it->second);
		}

		this->operator<< (tempKey);
		return this->operator<< (tempVal);
	}

	std::string str()
	{
		return os.str();
	}

public:
	std::ostringstream os;
};

class in_stream
{
public:

	in_stream(const std::string& s) : str(s), total(s.size())
	{
	}

	template<typename SerializableType>
	in_stream& operator>> (SerializableType& a)
	{
		int ret = ::deserialize(str, a);
		str = str.substr(ret);
		return *this;
	}

	template<typename BasicType>
	in_stream& operator>> (std::vector<BasicType>& a)
	{
		unsigned int len = 0;
		int ret = ::deserialize(str, len);
		str = str.substr(ret);

		for (unsigned int i = 0; i < len; ++i)
		{
			BasicType item;
			int size = ::deserialize(str, item);
			str = str.substr(size);
			a.push_back(item);
		}

		return *this;
	}

	template<typename BasicType>
	in_stream& operator>> (std::list<BasicType>& a)
	{
		std::vector<BasicType> temp;
		in_stream& ret = this->operator>> (temp);
		if (temp.size() > 0)
		{
			std::copy(temp.begin(), temp.end(), std::back_inserter(a));
		}

		return ret;
	}

	template<typename BasicType>
	in_stream& operator>> (std::set<BasicType>& a)
	{
		std::vector<BasicType> temp;
		in_stream& ret = this->operator>> (temp);
		if (temp.size() > 0)
		{
			for (size_t i = 0; i < temp.size(); ++i)
			{
				a.insert(temp[i]);
			}
		}

		return ret;
	}

	template<typename BasicType>
	in_stream& operator>> (std::unordered_set<BasicType>& a)
	{
		std::vector<BasicType> temp;
		in_stream& ret = this->operator>> (temp);
		if (temp.size() > 0)
		{
			for (size_t i = 0; i < temp.size(); ++i)
			{
				a.insert(temp[i]);
			}
		}

		return ret;
	}

	template<typename BasicTypeA, typename BasicTypeB>
	in_stream& operator>> (std::map<BasicTypeA, BasicTypeB>& a)
	{
		std::vector<BasicTypeA> tempKey;
		std::vector<BasicTypeB> tempVal;

		this->operator>> (tempKey);
		in_stream& ret = this->operator>> (tempVal);

		if (tempKey.size() > 0 && tempVal.size() == tempKey.size())
		{
			for (size_t i = 0; i < tempKey.size(); ++i)
			{
				a.insert(std::make_pair<BasicTypeA, BasicTypeB>(tempKey[i], tempVal[i]));
			}
		}

		return ret;
	}

	template<typename BasicTypeA, typename BasicTypeB>
	in_stream& operator>> (std::unordered_map<BasicTypeA, BasicTypeB>& a)
	{
		std::vector<BasicTypeA> tempKey;
		std::vector<BasicTypeB> tempVal;

		this->operator>> (tempKey);
		in_stream& ret = this->operator>> (tempVal);

		if (tempKey.size() > 0 && tempVal.size() == tempKey.size())
		{
			for (size_t i = 0; i < tempKey.size(); ++i)
			{
				//a.insert(std::make_pair<BasicTypeA, BasicTypeB>(tempKey[i], tempVal[i]));
				a.emplace(tempKey[i], tempVal[i]);
			}
		}

		return ret;
	}

	int size()
	{
		return static_cast<int>(total - str.size());
	}

protected:
	std::string str;
	size_t total;
};

////////////////////////////////////////////
//Serialize for custom class object
//If your class object want to be serialized,
//Plese derive for this base class
///////////////////////////////////////////

class Serializable
{
public:
	virtual std::string serialize() = 0;
	virtual int deserialize(const std::string&) = 0;
};

///////////////////////////////////////////
//!!!!!!!!!!!!!
//!!   NOTE  !!
//!!!!!!!!!!!!!
//Now,we can't serialize pointer type,likes
//char* var,char var[],int *var,int var[]
//etc,if you need to serialize array data,
//we suggest you use std::vector instead.
///////////////////////////////////////////

#endif
