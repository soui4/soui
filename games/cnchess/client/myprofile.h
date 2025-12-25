#ifndef MYPROFILE_H
#define MYPROFILE_H

#include <vector>
#include <core/SSingleton.h>
#include <protocol.h>
#include <memory>

/**
 * @brief 用户个人信息管理类
 * 
 * 该类使用单例模式管理当前用户的个人信息，包括用户ID、桌号、座位号、姓名、性别和头像等信息。
 * 通过SSingleton模板实现单例模式，确保全局只有一个实例。
 */
class MyProfile : public SSingleton<MyProfile>
{
public:
    /**
     * @brief 构造函数
     */
    MyProfile(void);
    
    /**
     * @brief 析构函数
     */
    ~MyProfile(void);
    
    /**
     * @brief 设置桌号
     * @param nTableId 桌号
     */
    void SetTableId(int nTableId);
    
    /**
     * @brief 获取桌号
     * @return 桌号
     */
    int GetTableId();
    
    /**
     * @brief 设置座位索引
     * @param nSeatIndex 座位索引
     */
    void SetSeatIndex(int nSeatIndex);
    
    /**
     * @brief 获取座位索引
     * @return 座位索引
     */
    int GetSeatIndex();
    
    /**
     * @brief 设置性别
     * @param nSex 性别
     */
    void SetSex(int nSex);

    /**
     * @brief 设置用户名
     * @param strName 用户名
     */
    void SetName(const SStringT &strName);
    
    /**
     * @brief 获取用户名
     * @return 用户名
     */
    SStringT GetName() const;
    
    /**
     * @brief 设置头像数据
     * @param pData 头像数据指针
     * @param nSize 数据大小
     */
    void SetAvatarData(const void *pData, int nSize);
    
    /**
     * @brief 获取头像数据
     * @return 头像数据的共享指针
     */
    const std::shared_ptr<std::vector<BYTE> > GetAvatarData() const;
    
    /**
     * @brief 获取用户信息
     * @return 用户信息的共享指针
     */
    std::shared_ptr<GS_USERINFO> GetUserInfo(){
        return m_pUserInfo;
    }
    
    /**
     * @brief 获取用户ID
     * @return 用户ID
     */
    uint32_t GetUID() const;
    
    /**
     * @brief 设置用户ID
     * @param uid 用户ID
     */
    void SetUID(uint32_t uid);
  
private:
    std::shared_ptr<GS_USERINFO> m_pUserInfo;     ///< 用户信息
    int m_nTableId;                               ///< 桌号
    int m_nSeatIndex;                             ///< 座位索引
    std::shared_ptr<std::vector<BYTE> > m_avatar; ///< 头像数据
};

#endif//MYPROFILE_H