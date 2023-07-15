/*******************************************************************************
  
  此类用于中小型ERP系统开发中前后台参数打包，主要在易用性和性能上做了处理

  File Name: ParamObject.h
  Created Date:  2008-01-20
  Author: wuding
  Version: 5.0
  Last Version Date: 2016-11-05 
  Modified History:
  version Date      Author    Description
  -----------------------------------------------------------------
  2008-01-20  wuding   version 1.0  加入基本数据类型，提供基本功能
  2008-07-23  wuding   version 2.0   对内存数据做了优化***
  2009-09-05  wuding   version 3.0   去掉了map，采用自定义的二叉树和哈唏表处理
  2009-11-05  wuding   version 4.0   去掉了自定义的二叉树和哈唏表处理，用map处理,并加入修改以前的c字节数组风格，采用blob类来处理字符流
  2010-06-13  wuding   version 5.0   将字符串存储加入'\0'结束符，加快了查询速度，极大的减少了内存分配产生的内存碎片
  2010-06-21  wuding   version 6.0   将字符串存储中'\0'结束符去掉
  2012-02-27  wuding   version 7.0   加入xml序列化支持,对于非普通数据类型的数据进行Base64编码以支持xml结点显示
  2016-11-05  wuding   version 8.0   
  改了一下，本来想扣chrome的ParamObject来用的，结果找不到，就直接拿以前c++Builder写的东西改改用吧，基本也够用
  这次主要去掉了关键的业务协议相关的东西，将dataset拿掉，去掉了xml拿掉,简化ERPBlob为BlobBuffer


********************************************************************/

#pragma once

#ifndef _FileStream_H
#define _FileStream_H


#include "BlobBuffer.h"
#include "ErpExcept.h"
#include <string>
/******************************************************************************
*类名：  ParamObject
*类说明：本类是一个参数处理类，在实际处理过程中存在界面层与实际逻辑层的参数传递，
*         特别是采用三层结构通过网络进行请求处理时，界面层与逻辑处理层存在大量的
         参数和数据交换，为了简化设计，本系统的设计采用valuebyname的方式统一处理
         参数问题，本类实现了各种参数的保存、传递、查找等操作。
******************************************************************************/
/*
/////////////////////////////////////////////////////////////////////////////
unsigned long ptr;
---------------以下为参数类的实现，
注意：此参数类只适合于少量参数，当参数个数大于1000时，请采用其它方法---
为了兼容以前的PutString同一个参数可以PutString多次，此类中为了快速存放数据，
如果同个参数PutString多次，会将最后一个参数置为激活状态，但不会删除以前PutString进去的数据。
多次put只支持字符串函数PutString，不支持流PutStream!!!
因此尽量不要多次PutString多个参数.
////////////////////////////////////////////////////////////////////////
*/
typedef struct _ParamNode
{               
  unsigned char type;  
  unsigned int  length;
  char name[256]; //参数名不能超过256．参数名不能超过256．参数名不能超过256 重要的事情说三遍
}ParamNode;
  

class  ParamObject
{
	protected:
		std::map<std::string, unsigned long>::iterator itr;
        std::map<std::string,unsigned long> paramNames;    //参数名,参数在对象中的位置，即参数索引
        BlobBuffer mParamsBuffer;            //参数字节流

        int getParamPos(const char *paramName,unsigned char **stream,unsigned int &length,unsigned char &pNodeType,const unsigned char dataType=PARAMS_STREAM); //取字节流位置
    public:
        ParamObject();
        virtual ~ParamObject();
        enum eParamDataType
        {
           PARAMS_STRING =0,PARAMS_INTEGER=1,PARAMS_LONG =2,PARAMS_FLOAT=3,PARAMS_DOUBLE =4,
           PARAMS_STREAM=5,PARAMS_BLOB=6,PARAMS_DATASET =7,PARAMS_ParamObject =8,
           PARAMS_UNVALID_TYPE=9
        };
        void ReName(const char *src_paramname, const char *dst_paramname);  //对指定参数改名
        void Refresh();  //刷新数据，重新排列，清空无效节点
        void Clear();   //清空参数对象中的内容
        void Clone(ParamObject *);        //从另一个对象中拷贝，相当于拷贝构造函数
        void AppendForm(ParamObject *other);  //从另一个对象中拷贝到本对象的尾部
        
        //从Blob对象中反序列化
		void LoadFromBlob(BlobBuffer &);

        //序列化到Blob对象中
		void SaveToBlob(BlobBuffer &);
  
        //从内存流中加载
        void LoadFromStream(const unsigned char *,unsigned int );

        //从文件中加载
        void LoadFromFile(const char *file_name);

        //保存到文件中
        void SaveToFile(const char *file_name);

        const unsigned char *getBytePtr()  ;       //返回内部字节流的指针
        unsigned int getByteLength() ;  //返回当前有效字节长度
        unsigned int getAllByteLength() ; //返回当前分配字节总长度
        unsigned int getParamCount() ; //返回有效参数个数
    public:         
        bool isExist(const char *paramName);    //判断是否有送某参数
        void remove(const char *paramName);

        //set param value
        void putInteger(const char *paramName,int paramValue);          //保存整型参数
        void putIntegerNull(const char *paramName,int paramValue);          //保存整型参数
        void putString(const char *paramName,const char* paramvalue,unsigned char dataType=PARAMS_STRING);   //保存字符串参数
        void putLong(const char *paramName,long paramValue);            //保存长整型参数
        void putFloat(const char *paramName,float paramValue,  const int Precision=6); //  //保存浮点型串参数
        void putDouble(const char *paramName,double paramValue,  const int Precision=6); // //保存双精度参数
        void putStream(const char *paramName,const unsigned char *paramValue,unsigned int len, bool redu_check=true,const unsigned char dataType=PARAMS_STREAM);
		void putBlob(const char *paramName, BlobBuffer &blob);
		// void putDataSet(const char *paramName,DataSet &dataset);            //保存序列参数
        void putParamObject(const char *paramName,ParamObject &object);            //支持参数对象嵌套


        //get param value
        int getInteger(const char *paramName);                         //读取整型参数
        long getLong(const char *paramName);                            //读取长整型参数
        std::string getString(const char *paramName,unsigned char dataType=PARAMS_STRING);                         //读取字符串参数 
        float getFloat(const char *paramName );                        //读浮点型参数
        double getDouble(const char *paramName );                       //读双精度参数
        //void getDataSet(const char *paramName,DataSet &dataset);         //读取序列参数
        void getParamObject(const char *paramName,ParamObject &obj);         //读取嵌套的参数对象
		bool getBlob(const char *paramName, BlobBuffer &blob, bool bException=true);
		unsigned char *getBlobEx(const char *paramName, unsigned int  &length);

        unsigned int First();                           //当前参数指向第一个参数
        unsigned int Next(unsigned int pos);                            //当前参数指向下一个参数
        bool Eof(unsigned int pos);                             //是否已经到最后一个参数
        std::string getParamName(unsigned int pos);                  //得到当前参数的名称
        int getParamType(unsigned int pos);  //得到当前参数的类型
        bool getParamValue(unsigned int pos,unsigned char **value,int *length);      //得到当前参数的值
        bool getParamValue(unsigned int pos,BlobBuffer *pBlob,unsigned char fillEndChar);
        std::string getParamValue(unsigned int pos);  //得到当前的参数的字符串值，如果当前参数是流就返回空
        void InitParamNames(); //初始化名字，重建索引
}; 

class CFileStream : public ParamObject
{
private:
	CFileStream(const CFileStream &);
	CFileStream &operator=(const CFileStream &);
public:
	CFileStream();
	~CFileStream();
public:
	bool PutFile(const char *fileName, BlobBuffer &blob);            //加入文件
	bool GetFile(const char *fileName, BlobBuffer &blob);            //加入文件
	unsigned long GetFileSize(const char *fileName);
	const unsigned char *GetFilePtr(const char *fileName);
	int GetFileCount();
	unsigned int GetAllSize();
	const unsigned char *GetAllBasePtr();
};

#endif
