#include "FileStream.h"
#include <time.h>
#include <memory.h>
#include <vector>
#include <algorithm>
#include <string.h>


/*******************************************************************************
类ParamObject实现代码
*******************************************************************************/

static std::string tolowerstring(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

ParamObject::ParamObject()
{
}
ParamObject::~ParamObject()
{ 
}
//将另外一个对象的所有参数加到这个对象中
void ParamObject ::AppendForm(ParamObject *other)
{
      if(other!=NULL)
      { 
            //将新对象有效字节数拷贝到本对象中
            mParamsBuffer.AppendBlobContent(other->mParamsBuffer.GetBlobPtr(),other->mParamsBuffer.GetBlobLength());

            //生成参数新的索引 

            std::map<std::string,unsigned long>::iterator itrOther;
            std::map<std::string,unsigned long>::iterator itrFind;
            for(itrOther= other->paramNames.begin(); itrOther!=other->paramNames.end(); )
            {
                //将每一个参数的名字和它对应字符流中超始位置放到map中
                //如果本对象已有此参数，就将本对象中参数置为PARAMS_UNVALID_TYPE无效节点
                itrFind = paramNames.find(itrOther->first);
                if( itrFind!= paramNames.end())
                {
                    //将本对象中已有的同名称节点设置为无效节点
                    ParamNode *pNode;
                    pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+ itrFind->second  );
                    pNode->type      = PARAMS_UNVALID_TYPE;

                    //并更新它的位置信息
                    itrFind->second  = mParamsBuffer.GetBlobLength() + itrOther->second ;
                }
                else
                { 
                    paramNames.insert(std::map<std::string,unsigned long>::value_type(itrOther->first,mParamsBuffer.GetBlobLength()+itrOther->second ));
                }
                itrOther++;
            }

            Refresh();
      }
}

void ParamObject::ReName(const char *src_paramname, const char *dst_paramname)
{ 
      std::map<std::string,unsigned long>::iterator itr;

      //如果dst_paramname已经存在，则不允许改成dst_paramname
      itr = paramNames.find(dst_paramname);
      if(itr!=paramNames.end())
      {
         throw ERPException(EERROR,SYSTEM_ERROR,"can,t rename [%s] to [%s] , param [%s] exist!",src_paramname,dst_paramname,dst_paramname);
      }

      itr = paramNames.find(src_paramname);
      if(itr==paramNames.end())
      {
         throw ERPException(EERROR,SYSTEM_ERROR,"can,t rename [%s] to [%s] , param [%s] not exist!",src_paramname,dst_paramname,src_paramname);
      }



      unsigned int node_start_pos;
      ParamNode rNode;

      BlobBuffer TmpParamsBuffer;
      //先计算位置，找到原结点的数据
      node_start_pos   = itr->second;
      memcpy((unsigned char*)&rNode, (unsigned char*)mParamsBuffer.GetBlobPtr()+node_start_pos, sizeof(ParamNode));

      paramNames.erase(itr);

      
      //更新索引值
      //paramNames.Insert(key,length);
      paramNames.insert(std::map<std::string,unsigned long>::value_type(dst_paramname,node_start_pos));

      //更新数据值
 
      memcpy(rNode.name, dst_paramname,sizeof(rNode.name));
      memcpy((unsigned char*)mParamsBuffer.GetBlobPtr()+node_start_pos, (unsigned char*)&rNode ,sizeof(ParamNode));

}

/*
----刷新数据，重新排列，清空无效节点,
    分配一块大内存，然后把数据一个一个拷过去
*/
void ParamObject::Refresh()
{
      if(getParamCount()<1)
       return;

      BlobBuffer TmpParamsBuffer;  

      unsigned int start_pos =0;
      unsigned int end_pos   =0;
      ParamNode *pNode;

      //BiNode *pFindBiNode;
      for( ; start_pos< mParamsBuffer.GetBlobLength() ; start_pos = end_pos)
      {
          //计算位置
          pNode      = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr() +start_pos  );
          end_pos   += sizeof(ParamNode)+pNode->length;

          //如果是有效节点，就生成新的参数索引，并拷贝数据进临时缓冲中
          if(pNode->type!=PARAMS_UNVALID_TYPE)
          { 
               //首先更新所有有效结点的索引
               itr = paramNames.find(pNode->name);
               if(itr != paramNames.end())
                 itr->second    = TmpParamsBuffer.GetBlobLength();
                
               //再拷贝数据到新的大内存中--
               TmpParamsBuffer.AppendBlobContent(mParamsBuffer.GetBlobPtr()+start_pos , end_pos-start_pos);

          }

      }

      mParamsBuffer.SetBlobContent(TmpParamsBuffer.GetBlobPtr(),TmpParamsBuffer.GetBlobLength() );
      First();
}
//---删除某个参数-----------
void ParamObject::remove(const char *paramName)
{
      std::map<std::string,unsigned long>::iterator itr;
      itr = paramNames.find(paramName);
      if(itr==paramNames.end())
      {
        return;
      }

      unsigned int node_start_pos; 
      ParamNode rNode;

      BlobBuffer TmpParamsBuffer;
      //先计算位置
      node_start_pos   = itr->second;
      memcpy((unsigned char*)&rNode, (unsigned char*)mParamsBuffer.GetBlobPtr()+node_start_pos, sizeof(ParamNode)); 
      
      TmpParamsBuffer.SetBlobContent(mParamsBuffer.GetBlobPtr(),node_start_pos);
      TmpParamsBuffer.AppendBlobContent(mParamsBuffer.GetBlobPtr()+ node_start_pos+sizeof(ParamNode)+ rNode.length,mParamsBuffer.GetBlobLength()-rNode.length);
      mParamsBuffer.SetBlobContent(TmpParamsBuffer.GetBlobPtr(),TmpParamsBuffer.GetBlobLength());
      
      paramNames.erase(itr);


      //重建此参数后面所有参数的参数索引
      ParamNode *pNode;
      for( ; node_start_pos<mParamsBuffer.GetBlobLength(); )
      {
              //计算位置
              pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+node_start_pos );
              std::map<std::string,unsigned long>::iterator itr;
              itr = paramNames.find(pNode->name);
              if(itr!= paramNames.end())
              {
                 itr->second = node_start_pos;
              }
              node_start_pos   += sizeof(ParamNode)+pNode->length;
      }


}
//清空参数对象中的内容
void ParamObject::Clear()
{ 
    paramNames.clear();
    mParamsBuffer.ClearContent();
}
//保存整型参数
void ParamObject::putInteger(const char *paramName,int paramValue)
{
    char sValue[64];
    sprintf(sValue, "%d",paramValue );
    putString( paramName,sValue,PARAMS_INTEGER);
}

void ParamObject::putIntegerNull(const char *paramName,int paramValue)
{
    char sValue[64];
    if( paramValue==0 )
          sValue[0] = '\0';
    else
          sprintf(sValue, "%d",paramValue );
    putString( paramName,sValue,PARAMS_INTEGER );
}

//保存长整型参数
void ParamObject::putLong(const char *paramName,long paramValue)
{
      char sValue[64];   
      sprintf(sValue, "%ld",paramValue );
      putString( paramName,sValue ,PARAMS_LONG);
}

//保存浮点型串参数
void ParamObject::putFloat(const char *paramName,float paramValue,
    const int Precision)
{
      char sFmtString[25];
      char sValue[64];
      sprintf(sFmtString, "%%.%df",Precision );
      sprintf(sValue, sFmtString,paramValue );
      putString( paramName,sValue,PARAMS_FLOAT);
}

//保存双精度参数
void ParamObject::putDouble(const char *paramName,double paramValue,
    const int Precision)
{
      char sFmtString[25];
      char sValue[64];

      sprintf(sFmtString, "%%.%dlf",Precision );
      sprintf(sValue, sFmtString,paramValue );
      putString( paramName,sValue,PARAMS_DOUBLE);
}

/*
保存字符串参数到对象中，
注，对于字符串参数，02版本为了提高效率，将'\0'字符也存储到序列中了
这样的话在GetString时就可以不用拷贝字符串单独处理
*/

void ParamObject::putString(const char *paramName,const char *paramValue,unsigned char dataType/*=PARAMS_STRING*/)
{
      unsigned int start_pos;
   
      std::map<std::string,unsigned long>::iterator itr;
      itr = paramNames.find(paramName);
      if(itr!= paramNames.end())
      {
          start_pos =itr->second;
          
          ParamNode *pNode; 
          pNode       = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr() +start_pos  );
          pNode->type = PARAMS_UNVALID_TYPE;  //将原有节点设置为无效节点
          paramNames.erase(itr);
      }                                                          

      putStream(paramName,(const unsigned char *)paramValue,strlen(paramValue),false,dataType);
}

//保存参数流到对象中
void ParamObject::putStream(const char *paramName,const unsigned char *paramValue,unsigned int len
                           ,bool redu_check,const unsigned char dataType)
{ 
      if(redu_check)
      {
          std::map<std::string,unsigned long>::iterator itr;
          itr = paramNames.find(paramName);
          if(itr!=paramNames.end() &&  dataType==PARAMS_STREAM)
          {
             throw ERPException(EERROR,SYSTEM_ERROR,"已经有参数流[%s]，不能重复存放此参数流！",paramName);
          }
      }

      //新建索引
      //paramNames.Insert(key,length); 
	  unsigned long size = mParamsBuffer.GetBlobLength();
      paramNames.insert(std::map<std::string,unsigned long>::value_type(paramName,size));

      ParamNode cNode;
      memset(&cNode,'\0',sizeof(ParamNode));
      memset(cNode.name,'\0',sizeof(cNode.name));
      cNode.length = len;
      cNode.type   = dataType;
	  strncpy(cNode.name, paramName, sizeof(cNode.name));
      
      mParamsBuffer.AppendBlobContent((unsigned char *)&cNode,sizeof(ParamNode));
      mParamsBuffer.AppendBlobContent(paramValue,cNode.length); 
}


void ParamObject::putBlob(const char *paramName,BlobBuffer &blob)
{
      putStream(paramName,blob.GetBlobPtr() ,blob.GetBlobLength(),true,PARAMS_BLOB );
}

/*
//保存序列参数
void ParamObject::putDataSet(const char *paramName,DataSet &dataset)
{
      BlobBuffer blobBuffer;
      dataset.Serialize(blobBuffer) ;

      putStream( paramName,blobBuffer.GetBlobPtr(),blobBuffer.GetBlobLength(),true,PARAMS_DATASET );
}
*/
//支持参数对象嵌套
void ParamObject::putParamObject(const char *paramName,ParamObject &object)
{
      BlobBuffer blobBuffer;
	  object.SaveToBlob(blobBuffer);
      putStream( paramName,blobBuffer.GetBlobPtr(),blobBuffer.GetBlobLength(),true,PARAMS_ParamObject );
}

//读取整型参数
int  ParamObject::getInteger(const char *paramName)
{
      return(atoi(getString(paramName,PARAMS_INTEGER).c_str()));
}

//读取长整型参数
long ParamObject::getLong(const char *paramName)
{
      return(atol(getString(paramName,PARAMS_LONG).c_str()));
}

//取指定参数在对象中的指针位置
int ParamObject::getParamPos(const char *paramName,unsigned char **stream,unsigned int &length,unsigned char &pNodeType,const unsigned char dataType)
{
      unsigned int pos;
      *stream =0;
   
      std::map<std::string,unsigned long>::iterator itr;
	  std::string sName(paramName);
	  itr = paramNames.find(sName);
      if(itr== paramNames.end())
      {
          return 0;
      }
      pos =itr->second;

      ParamNode *pNode;
      pNode    = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
	   
	  if (0 == strcmp(pNode->name, paramName))
	  {
		  *stream = (unsigned char *)mParamsBuffer.GetBlobPtr() + pos + sizeof(ParamNode);
		  pNodeType = pNode->type;

		  length = pNode->length;
	  }
	  else
		  length = 0;

      return (0==*stream)? 0:1;
}

//读字符串型参数，不需考虑结束符'\0'，因为put的时候已将结束符'\0'存入
std::string ParamObject::getString(const char *paramName,unsigned char dataType/*=PARAMS_STRING*/)
{
      std::string  sTemp="";
      unsigned char tail_char;
      unsigned int  length;
      unsigned char NodeType;
      unsigned char *pParamPos;
      if(0!= getParamPos(paramName,&pParamPos,length,NodeType,dataType))
      {
           //因PutStream时未处理字符串的结束符'\0'，在这里需要加上
           tail_char             = *(pParamPos + length);
           *(pParamPos + length) ='\0';
           sTemp                 = (char *)(pParamPos);
           *(pParamPos + length) =tail_char;
      }
      else
         throw ERPException(EERROR,SYSTEM_ERROR,"parameter[%s]not found！",paramName);

      return sTemp;
}


bool ParamObject::getBlob(const char *paramName, BlobBuffer &blob,bool bException)
{ 
	  unsigned int  length=0;
	  unsigned char *pParamPos = getBlobEx(paramName, length);
	  if (NULL  == pParamPos)
      {
		  if (0 == length)
		  {
			  if (bException)
				  throw ERPException(EERROR, SYSTEM_ERROR, "blob parameter[%s]not found！", paramName);
			  else
				  return false;
          }
		  else
		  {
			  if (bException)
				  throw ERPException(EERROR, SYSTEM_ERROR, "not a blob parameter[%s]！", paramName);
			  else
				  return false;
		  }
      }
	  else
		  blob.SetBlobContent(pParamPos, length); 
	  return true;
}

unsigned char *ParamObject::getBlobEx(const char *paramName, unsigned int  &length)
{
	unsigned char NodeType;
	unsigned char *pParamPos;
	if (0 != getParamPos(paramName, &pParamPos, length, NodeType, PARAMS_BLOB))
	{
		if (PARAMS_BLOB == NodeType)
		{
			return pParamPos;
		}
		else
			return NULL;
	}
	else
		return NULL;
}

//判断是否有送某参数
bool ParamObject::isExist(const char *paramName)
{
      //return paramNames.Find(DataSet::MyHash(paramName,strlen(paramName)));
      return (paramNames.find(paramName)!=paramNames.end());
  
}
//读浮点型参数
float  ParamObject::getFloat(const char *paramName )
{
     return (float)(atof(getString(paramName,PARAMS_FLOAT).c_str()));
}

//读双精度参数
double ParamObject::getDouble(const char *paramName )
{
     return(atof(getString(paramName,PARAMS_DOUBLE).c_str()));
}


/*
//读取序列参数
void ParamObject::getDataSet(const char *paramName,DataSet &dataset )
{
      unsigned int len;
      unsigned char NodeType;
      unsigned char *stream;
      if(getParamPos(paramName,&stream,len,NodeType,PARAMS_DATASET))
      {
          if(PARAMS_DATASET==NodeType)
          {
                try
                {
                    dataset.UnSerialize(stream,len);
                    dataset.setName(paramName);
                }
                catch(ERPException &e )
                { 
                    char c_time[128];
                    tm *temptm;
                    time_t temptime;
                    temptime= time(0);
                    temptm = localtime(&temptime);
                    sprintf(c_time,"%d年%02d月%02d日 %02d:%02d:%02d",
                           temptm->tm_year+1900,temptm->tm_mon+1,temptm->tm_mday,
                           temptm->tm_hour,temptm->tm_min,temptm->tm_sec);
                    WriteLog("ParamObject.log","a+",c_time,dataset.GetFieldNames().c_str());
                    sprintf(c_time,"%d%02d%02d%02d%02d%02d_异常数据.log",
                            temptm->tm_year+1900,temptm->tm_mon+1,temptm->tm_mday,
                            temptm->tm_hour,temptm->tm_min,temptm->tm_sec );
                    SaveToFile(c_time);
                    throw e;
                }
          }
          else
            throw ERPException(EERROR,SYSTEM_ERROR,"DataSet parameter[%s]not found！",paramName);
      }
      else
      {
         throw ERPException(EERROR,SYSTEM_ERROR,"DataSet parameter[%s]not found！",paramName);
      }
}
*/
//读取嵌套的参数对象
void ParamObject::getParamObject(const char *paramName,ParamObject &obj)
{
      unsigned int len;
      unsigned char NodeType;
      unsigned char *stream;
      if(getParamPos(paramName,&stream,len,NodeType,PARAMS_ParamObject))
      {
          if(PARAMS_ParamObject==NodeType)
          {
                try
                {
                    obj.LoadFromStream((const unsigned char *)stream,len);
                }
                catch(ERPException &e )
                { 
                    char c_time[128];
                    tm *temptm;
                    time_t temptime;
                    temptime= time(0);
                    temptm = localtime(&temptime);
                    sprintf(c_time,"%d年%02d月%02d日 %02d:%02d:%02d",
                           temptm->tm_year+1900,temptm->tm_mon+1,temptm->tm_mday,
                           temptm->tm_hour,temptm->tm_min,temptm->tm_sec);
                    sprintf(c_time,"%d%02d%02d%02d%02d%02d_异常数据.log",
                            temptm->tm_year+1900,temptm->tm_mon+1,temptm->tm_mday,
                            temptm->tm_hour,temptm->tm_min,temptm->tm_sec );
                    SaveToFile(c_time);
                    throw e;
                }
          }
          else
            throw ERPException(EERROR,SYSTEM_ERROR,"ParamObject parameter[%s]not found！",paramName);
      }
      else
      {
         throw ERPException(EERROR,SYSTEM_ERROR,"ParamObject parameter[%s]not found！",paramName);
      }
}

//当前参数指向第一个参数
unsigned int ParamObject::First()
{
	return 0;
//      curr_pos=0;
}
//当前参数指向下一个参数
unsigned int ParamObject::Next(unsigned int pos)
{
      if(pos>=mParamsBuffer.GetBlobLength())
		return -1;
       
      ParamNode *pNode; 
      pNode     = (ParamNode *)(  (unsigned char*)mParamsBuffer.GetBlobPtr()+pos  );
      pos +=sizeof(ParamNode)+pNode->length;
      if(pNode->type==PARAMS_UNVALID_TYPE)
         return Next(pos);
	  else
		  return pos;
}
//判断是否已到了结尾
bool ParamObject::Eof(unsigned int pos)
{
    return (mParamsBuffer.GetBlobLength()<=pos);
}
//得到当前参数的名称
std::string ParamObject::getParamName(unsigned int pos)
{
      ParamNode *pNode;
      pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
      return (pNode->name);
}
//得到当前参数的名称
int ParamObject::getParamType(unsigned int pos)
{
      ParamNode *pNode;
      pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
      return (pNode->type);
}
//得到当前参数的值
bool ParamObject::getParamValue(unsigned int pos,unsigned char **value,int *length)
{
      ParamNode *pNode;
      pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
      if(pNode->length<1)
      {
       return false;
      }
      *value=new unsigned char[pNode->length];
      memcpy((unsigned char *)*value,(unsigned char *)mParamsBuffer.GetBlobPtr()+pos+sizeof(ParamNode),pNode->length);
	  *length = pNode->length;
      return true;
}

bool ParamObject::getParamValue(unsigned int pos,BlobBuffer *pBlob,unsigned char fillEndChar)
{
     ParamNode *pNode;
      pNode = (ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
      if(pNode->length<1)
      {
       return false;
      }
      pBlob->SetBlobContent((unsigned char *)mParamsBuffer.GetBlobPtr()+pos+sizeof(ParamNode),pNode->length);
      if(1==fillEndChar)
        pBlob->AppendBlobContentChar('\0');
	  return true;
}
//得到当前的参数的字符串值，如果当前参数是流就返回空
std::string ParamObject::getParamValue(unsigned int pos)
{
      ParamNode *pNode;
      pNode =(ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+pos );
      if(PARAMS_STREAM==pNode->type)
       return  "stream";

	  return std::string((char*)mParamsBuffer.GetBlobPtr() + pos + sizeof(ParamNode), pNode->length);
}
//--返回实际字节流指针-----
const unsigned char *ParamObject::getBytePtr()
{
      return mParamsBuffer.GetBlobPtr();
}
//---返回实际字节流大小----
unsigned int ParamObject::getByteLength()
{
      return mParamsBuffer.GetBlobLength();
}
//返回当胶字节总长度
unsigned int ParamObject::getAllByteLength()
{
     return mParamsBuffer.GetBlobBufferLength();
}
//clone
void ParamObject::Clone(ParamObject *pObject)
{
      mParamsBuffer.SetBlobContent(pObject->mParamsBuffer.GetBlobPtr(),pObject->mParamsBuffer.GetBlobLength());
      InitParamNames();
}

//从Blob对象中反序列化
void ParamObject::LoadFromBlob(BlobBuffer &blob)
{
    LoadFromStream(blob.GetBlobPtr(),blob.GetBlobLength());
}

//序列化到Blob对象中
void ParamObject::SaveToBlob(BlobBuffer &blob)
{
    blob.SetBlobContent(mParamsBuffer.GetBlobPtr(),mParamsBuffer.GetBlobLength()); 
}

 
/*/////////////////////////////////////////////////////////////////////////////
 load param from the stream buffer 
 initial the paramnames 
/////////////////////////////////////////////////////////////////////////////*/
void ParamObject::LoadFromStream(const unsigned char*in_buffer,unsigned int len)
{
      mParamsBuffer.SetBlobContent(in_buffer,len); 
      InitParamNames();
}
//load the ParamObject from the file which has the ParamObject format 
void ParamObject::LoadFromFile(const char *file_name)
{                      
      mParamsBuffer.LoadBlobFromFile(file_name);
      InitParamNames();
}
//save the ParamObject as the ParamObject format
void ParamObject::SaveToFile(const char *file_name)
{ 
      mParamsBuffer.SaveBlobToFile(file_name);
}

/*/////////////////////////////////////////////////////////////////////////////
----regenerate the param names and the index ---
////////////////////////////////////////////////////////////////////////////*/
void ParamObject::InitParamNames()
{
      ParamNode *pNode;
      unsigned int curr_pos=0;
      paramNames.clear(); 
      //unsigned int key;
      std::map<std::string,unsigned long>::iterator itr;    
      while(curr_pos< mParamsBuffer.GetBlobLength())
      {
          pNode =(ParamNode *)( (unsigned char*)mParamsBuffer.GetBlobPtr()+curr_pos );

          itr = paramNames.find(pNode->name);
          if(itr != paramNames.end())
             paramNames.erase(itr);
          paramNames.insert(std::map<std::string,unsigned long>::value_type(pNode->name,curr_pos));
                    
          curr_pos+=sizeof(ParamNode)+pNode->length;
      }
}
//返回参数个数，如果有多次putstring同一个参数，则size无意义，
unsigned int ParamObject::getParamCount()
{
      return paramNames.size();
}


CFileStream::CFileStream(const CFileStream &other)
{
	 
}

CFileStream &CFileStream::operator=(const CFileStream &)
{
	return *this;
}

CFileStream::CFileStream()
{

}

CFileStream::~CFileStream()
{

}
//加入文件
bool CFileStream::PutFile(const char *fileName, BlobBuffer &blob)
{ 
	//改为不区分大小写吧，soui ui库大小写比较乱．．.	  
	std::string file_name(fileName);
	file_name = tolowerstring(file_name);

	this->putBlob(file_name.c_str(), blob);
	return true;
}

//加入文件
bool CFileStream::GetFile(const char *fileName, BlobBuffer &blob)
{
	//改为不区分大小写吧，soui ui库大小写比较乱．．.	  
	std::string file_name(fileName);
	file_name = tolowerstring(file_name);

#ifdef _DEBUG
	return getBlob(file_name.c_str(), blob, true);
#else
	return getBlob(file_name.c_str(), blob, false);
#endif
}

int CFileStream::GetFileCount()
{
	return getParamCount();
} 

const unsigned char *CFileStream::GetAllBasePtr()
{
	return getBytePtr();       //返回内部字节流的指针
}


unsigned long CFileStream::GetFileSize(const char *fileName)
{ 
	unsigned int  length = 0;
	if (NULL != getBlobEx(fileName, length))
		return length;
	else
		return 0;
}

const unsigned char *CFileStream::GetFilePtr(const char *fileName)
{
	unsigned int  length = 0;
	unsigned char *pParamPos = getBlobEx(fileName, length);
	return pParamPos;
}

unsigned int CFileStream::GetAllSize()
{
	return getByteLength();
}
