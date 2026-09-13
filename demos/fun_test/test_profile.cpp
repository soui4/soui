/**
 * swinx INI profile APIs (GetPrivateProfile* /WritePrivateProfile* roundtrip).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include <map>
#include <cstring>
#include <cstdio>

static std::string iniPath()
{
#ifdef _WIN32
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    char buf[320];
    sprintf(buf, "%sswinx_profile_test_%u.ini", tmp, GetCurrentProcessId());
#else
    char buf[256];
    sprintf(buf, "/tmp/swinx_profile_test_%u.ini", GetCurrentProcessId());
#endif
    return buf;
}

class ProfileTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path = iniPath();
        DeleteFileA(path.c_str());
    }
    void TearDown() override
    {
        DeleteFileA(path.c_str());
    }
    std::string path;
};

TEST_F(ProfileTest, write_then_read_string)
{
    EXPECT_TRUE(WritePrivateProfileStringA("sec1", "key1", "value1", path.c_str()));

    char buf[64] = {0};
    INT ret = GetPrivateProfileStringA("sec1", "key1", "none",
                                       buf, sizeof(buf), path.c_str());
    EXPECT_EQ(ret, 6);
    EXPECT_STREQ(buf, "value1");
}

TEST_F(ProfileTest, missing_entry_returns_default)
{
    WritePrivateProfileStringA("sec1", "key1", "value1", path.c_str());

    char buf[64] = {0};
    INT ret = GetPrivateProfileStringA("sec1", "no_such_key", "fallback",
                                       buf, sizeof(buf), path.c_str());
    EXPECT_EQ(ret, 8);
    EXPECT_STREQ(buf, "fallback");

    // missing section behaves the same
    memset(buf, 0, sizeof(buf));
    ret = GetPrivateProfileStringA("no_such_sec", "key1", "fallback",
                                   buf, sizeof(buf), path.c_str());
    EXPECT_EQ(ret, 8);
    EXPECT_STREQ(buf, "fallback");
}

TEST_F(ProfileTest, overwrite_entry)
{
    EXPECT_TRUE(WritePrivateProfileStringA("sec1", "key1", "first", path.c_str()));
    EXPECT_TRUE(WritePrivateProfileStringA("sec1", "key1", "second", path.c_str()));

    char buf[64] = {0};
    GetPrivateProfileStringA("sec1", "key1", "none", buf, sizeof(buf), path.c_str());
    EXPECT_STREQ(buf, "second");
}

TEST_F(ProfileTest, multiple_sections_and_keys)
{
    WritePrivateProfileStringA("a", "k", "va", path.c_str());
    WritePrivateProfileStringA("b", "k", "vb", path.c_str());
    WritePrivateProfileStringA("a", "k2", "va2", path.c_str());

    char buf[64] = {0};
    GetPrivateProfileStringA("a", "k", "", buf, sizeof(buf), path.c_str());
    EXPECT_STREQ(buf, "va");
    GetPrivateProfileStringA("b", "k", "", buf, sizeof(buf), path.c_str());
    EXPECT_STREQ(buf, "vb");
    GetPrivateProfileStringA("a", "k2", "", buf, sizeof(buf), path.c_str());
    EXPECT_STREQ(buf, "va2");
}

TEST_F(ProfileTest, get_int)
{
    WritePrivateProfileStringA("num", "n1", "123", path.c_str());
    WritePrivateProfileStringA("num", "neg", "-5", path.c_str());

    EXPECT_EQ(GetPrivateProfileIntA("num", "n1", 0, path.c_str()), 123u);
    EXPECT_EQ(GetPrivateProfileIntA("num", "neg", 0, path.c_str()), (UINT)-5);
    // missing key returns the default
    EXPECT_EQ(GetPrivateProfileIntA("num", "missing", 42, path.c_str()), 42u);
}

TEST_F(ProfileTest, section_lists_all_key_values)
{
    WritePrivateProfileStringA("sect", "k1", "v1", path.c_str());
    WritePrivateProfileStringA("sect", "k2", "v2", path.c_str());

    char buf[256];
    INT n = GetPrivateProfileSectionA("sect", buf, sizeof(buf), path.c_str());
    ASSERT_GT(n, 0);

    // entries are "key=value" strings separated by NUL, terminated by an
    // extra NUL; walk them and collect the pairs
    std::map<std::string, std::string> pairs;
    const char *p = buf;
    while (*p)
    {
        const char *eq = strchr(p, '=');
        ASSERT_TRUE(eq != NULL) << "entry without '=': " << p;
        pairs[std::string(p, eq - p)] = std::string(eq + 1);
        p += strlen(p) + 1;
    }
    EXPECT_EQ(pairs.size(), 2u);
    EXPECT_EQ(pairs["k1"], "v1");
    EXPECT_EQ(pairs["k2"], "v2");

    // missing section yields an empty (double-NUL) result
    memset(buf, 'x', sizeof(buf));
    n = GetPrivateProfileSectionA("no_such_sect", buf, sizeof(buf), path.c_str());
    EXPECT_EQ(n, 0);
    EXPECT_EQ(buf[0], '\0');
}

TEST_F(ProfileTest, section_names_lists_sections)
{
    WritePrivateProfileStringA("alpha", "k", "v", path.c_str());
    WritePrivateProfileStringA("beta", "k", "v", path.c_str());

    char buf[128];
    DWORD n = GetPrivateProfileSectionNamesA(buf, sizeof(buf), path.c_str());
    ASSERT_GT(n, 0u);

    // section names separated by NUL, terminated by an extra NUL
    bool sawAlpha = false, sawBeta = false;
    int count = 0;
    const char *p = buf;
    while (*p)
    {
        if (strcmp(p, "alpha") == 0)
            sawAlpha = true;
        else if (strcmp(p, "beta") == 0)
            sawBeta = true;
        count++;
        p += strlen(p) + 1;
    }
    EXPECT_EQ(count, 2);
    EXPECT_TRUE(sawAlpha);
    EXPECT_TRUE(sawBeta);
}

TEST_F(ProfileTest, struct_roundtrip_and_checksum)
{
    struct Blob
    {
        int a;
        double b;
        char tag[8];
    };
    Blob d;
    // zero the whole struct first: the profile checksum covers every byte,
    // including the padding between `a` and `b` that the assignments below
    // never touch (this is also what memcmp compares at the end)
    memset(&d, 0, sizeof(d));
    d.a = 42;
    d.b = 2.5;
    strcpy(d.tag, "swinx!");

    ASSERT_TRUE(WritePrivateProfileStructA("sstruct", "blob", &d, sizeof(d), path.c_str()));

    Blob r;
    memset(&r, 0, sizeof(r));
    EXPECT_TRUE(GetPrivateProfileStructA("sstruct", "blob", &r, sizeof(d), path.c_str()));
    EXPECT_EQ(memcmp(&d, &r, sizeof(d)), 0);

    // missing entry fails
    EXPECT_FALSE(GetPrivateProfileStructA("sstruct", "missing", &r, sizeof(d), path.c_str()));

    // a corrupted stored value must be rejected by the checksum validation
    FILE *fp = fopen(path.c_str(), "rb");
    ASSERT_TRUE(fp != NULL);
    std::string content;
    char ch;
    while (fread(&ch, 1, 1, fp) == 1)
        content += ch;
    fclose(fp);

    size_t pos = content.find("blob=");
    ASSERT_NE(pos, std::string::npos);
    pos += strlen("blob=");
    // flip one hex digit of the payload (not the trailing checksum char)
    ASSERT_GT(content.size(), pos + 2);
    content[pos + 1] = (content[pos + 1] == '0') ? '1' : '0';

    fp = fopen(path.c_str(), "wb");
    ASSERT_TRUE(fp != NULL);
    fwrite(content.data(), 1, content.size(), fp);
    fclose(fp);

    memset(&r, 0, sizeof(r));
    EXPECT_FALSE(GetPrivateProfileStructA("sstruct", "blob", &r, sizeof(d), path.c_str()));
}
