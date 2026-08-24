#ifndef MYPROFILE_H
#define MYPROFILE_H

#include <vector>
#include <core/SSingleton.h>
#include <protocol.h>
#include <memory>
#include <string/tstring.h>

/**
 * @brief 内置头像ID枚举
 */
namespace BuiltinAvatar {
    enum {
        CHAIR   = 0,    // 空座位占位头像
        MALE    = 1,    // 男性默认头像
        FEMALE  = 2,    // 女性默认头像
        AVATAR1 = 3,    // 24991.gif
        AVATAR2 = 4,    // 24993.gif
        COUNT,
        EXTERNAL = -1,  // 使用外部图片
    };
}

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
     * @brief 获取性别
     * @return 性别 (SEX_MALE/SEX_FEMALE/SEX_SECRET)
     */
    int GetSex() const { return m_pUserInfo->nSex; }

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
     * @brief 设置内置头像ID
     * @param nId 内置头像ID（>=0），或 BuiltinAvatar::EXTERNAL(-1) 表示外部图片
     */
    void SetAvatarId(int nId);

    /**
     * @brief 获取内置头像ID
     * @return 内置头像ID；返回 BuiltinAvatar::EXTERNAL(-1) 表示未设置或使用外部图片
     */
    int GetAvatarId() const { return m_nAvatarId; }

    /**
     * @brief 设置外部头像图片完整路径
     * @param strPath 外部图片路径
     */
    void SetAvatarPath(const SStringT &strPath);

    /**
     * @brief 获取外部头像图片完整路径
     * @return 外部图片路径（为空则未使用外部）
     */
    SStringT GetAvatarPath() const { return m_strAvatarPath; }
    
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
    std::shared_ptr<std::vector<BYTE> > m_avatar; ///< 头像二进制数据（外部图片/上传）
    int m_nAvatarId;                              ///< 内置头像ID (>=0) 或 -1
    SStringT m_strAvatarPath;                     ///< 外部图片完整路径（本地profile存储）
};

#endif//MYPROFILE_H