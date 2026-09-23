/**
 * swinx file API tests (CreateFile access modes, read/write/seek, dir ops).
 * Includes a regression test for the CreateFileA access-flag handling
 * (`dwDesiredAccess | GENERIC_x` vs `&`): with the bug, every file was opened
 * O_RDWR, so opening a read-only file with GENERIC_READ failed.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include "common.h"

static std::string tempPath(const char *name)
{
#ifdef _WIN32
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    char buf[320];
    sprintf(buf, "%sswinx_file_test_%u_%s", tmp, GetCurrentProcessId(), name);
#else
    char buf[256];
    sprintf(buf, "/tmp/swinx_file_test_%u_%s", GetCurrentProcessId(), name);
#endif
    return buf;
}

static std::wstring tempPathW(const wchar_t *name)
{
#ifdef _WIN32
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    wchar_t buf[320];
    swprintf(buf, 320, L"%sswinx_file_test_%u_%s", tmp, GetCurrentProcessId(), name);
#else
    wchar_t buf[256];
    swprintf(buf, 256, L"/tmp/swinx_file_test_%u_%S", GetCurrentProcessId(), name);
#endif
    return buf;
}

class FileTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dir = tempPath("dir");
        CreateDirectoryA(dir.c_str(), NULL);
    }
    void TearDown() override
    {
        // best-effort cleanup, ignore errors
        DeleteFileA((dir + "/a.txt").c_str());
        DeleteFileA((dir + "/b.txt").c_str());
        DeleteFileA((dir + "/moved.txt").c_str());
        chmod((dir + "/ro.txt").c_str(), 0644);
        DeleteFileA((dir + "/ro.txt").c_str());
        RemoveDirectoryA(dir.c_str());
    }
    std::string dir;
};

TEST_F(FileTest, create_write_read_roundtrip)
{
    std::string path = dir + "/a.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

    const char msg[] = "hello swinx file api";
    DWORD written = 0;
    EXPECT_TRUE(WriteFile(h, msg, sizeof(msg) - 1, &written, NULL));
    EXPECT_EQ(written, sizeof(msg) - 1);

    // read back from the beginning
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    char buf[64] = {0};
    DWORD readed = 0;
    EXPECT_TRUE(ReadFile(h, buf, sizeof(buf), &readed, NULL));
    EXPECT_EQ(readed, sizeof(msg) - 1);
    EXPECT_STREQ(buf, msg);

    CloseHandle(h);
}

TEST_F(FileTest, get_file_size)
{
    std::string path = dir + "/a.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    WriteFile(h, "0123456789", 10, &written, NULL);
    CloseHandle(h);

    h = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    EXPECT_EQ(GetFileSize(h, NULL), 10u);
    CloseHandle(h);
}

TEST_F(FileTest, set_file_pointer)
{
    std::string path = dir + "/a.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

    DWORD written = 0;
    WriteFile(h, "0123456789", 10, &written, NULL);

    SetFilePointer(h, 2, NULL, FILE_BEGIN);
    char buf[8] = {0};
    DWORD readed = 0;
    ReadFile(h, buf, 3, &readed, NULL);
    EXPECT_EQ(readed, 3u);
    EXPECT_STREQ(buf, "234");

    // pointer is now at 5; relative move +1 puts it at 6
    DWORD pos = SetFilePointer(h, 1, NULL, FILE_CURRENT);
    EXPECT_EQ(pos, 6u);
    memset(buf, 0, sizeof(buf));
    ReadFile(h, buf, 2, &readed, NULL);
    EXPECT_EQ(readed, 2u);
    EXPECT_STREQ(buf, "67");

    CloseHandle(h);
}

TEST_F(FileTest, open_nonexistent_fails)
{
    HANDLE h = CreateFileA((dir + "/no_such_file.txt").c_str(), GENERIC_READ, 0,
                           NULL, OPEN_EXISTING, 0, 0);
    EXPECT_EQ(h, INVALID_HANDLE_VALUE);
    if (h != INVALID_HANDLE_VALUE)
        CloseHandle(h);
}

// Regression: GENERIC_READ must map to a read-only open. With the old
// `dwDesiredAccess | GENERIC_READ` always-true bug the file was opened O_RDWR,
// which failed with EACCES on files without write permission (non-root).
TEST_F(FileTest, readonly_file_generic_read)
{
    std::string path = dir + "/ro.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    WriteFile(h, "readonly", 8, &written, NULL);
    CloseHandle(h);

    chmod(path.c_str(), 0444);

    h = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    EXPECT_TRUE(h != INVALID_HANDLE_VALUE);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[16] = {0};
        DWORD readed = 0;
        EXPECT_TRUE(ReadFile(h, buf, 8, &readed, NULL));
        EXPECT_EQ(readed, 8u);
        CloseHandle(h);
    }

    // writing to a read-only file must fail
    h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
    EXPECT_EQ(h, INVALID_HANDLE_VALUE);
    if (h != INVALID_HANDLE_VALUE)
        CloseHandle(h);

    chmod(path.c_str(), 0644);
}

TEST_F(FileTest, delete_file)
{
    std::string path = dir + "/a.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    CloseHandle(h);

    EXPECT_TRUE(DeleteFileA(path.c_str()));
    EXPECT_TRUE(GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES);
    // deleting again fails
    EXPECT_FALSE(DeleteFileA(path.c_str()));
}

TEST_F(FileTest, directory_ops)
{
    std::string sub = dir + "/sub";
    EXPECT_TRUE(CreateDirectoryA(sub.c_str(), NULL));
    // creating again fails
    EXPECT_FALSE(CreateDirectoryA(sub.c_str(), NULL));

    DWORD attr = GetFileAttributesA(sub.c_str());
    EXPECT_NE(attr, INVALID_FILE_ATTRIBUTES);
    EXPECT_TRUE(attr & FILE_ATTRIBUTE_DIRECTORY);

    EXPECT_TRUE(RemoveDirectoryA(sub.c_str()));
    EXPECT_EQ(GetFileAttributesA(sub.c_str()), INVALID_FILE_ATTRIBUTES);
}

TEST_F(FileTest, copy_and_move)
{
    std::string src = dir + "/a.txt";
    std::string dst = dir + "/b.txt";

    HANDLE h = CreateFileA(src.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    WriteFile(h, "copy me", 7, &written, NULL);
    CloseHandle(h);

    EXPECT_TRUE(CopyFileA(src.c_str(), dst.c_str(), FALSE));

    HANDLE hr = CreateFileA(dst.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(hr != INVALID_HANDLE_VALUE);
    char buf[16] = {0};
    DWORD readed = 0;
    ReadFile(hr, buf, 7, &readed, NULL);
    CloseHandle(hr);
    EXPECT_STREQ(buf, "copy me");

    std::string moved = dir + "/moved.txt";
    DeleteFileA(moved.c_str());
    EXPECT_TRUE(MoveFileA(dst.c_str(), moved.c_str()));
    EXPECT_EQ(GetFileAttributesA(dst.c_str()), INVALID_FILE_ATTRIBUTES);
    EXPECT_NE(GetFileAttributesA(moved.c_str()), INVALID_FILE_ATTRIBUTES);
}

// Regression: CopyFileA with bFailIfExists=TRUE must fail when the
// destination exists. The old code folded the comparison into the
// assignment (`DWORD attr = GetFileAttributesA(...) != INVALID`),
// so the existence check never triggered and the destination was
// silently overwritten.
TEST_F(FileTest, copy_file_fail_if_exists)
{
    std::string src = dir + "/a.txt";
    std::string dst = dir + "/b.txt";

    HANDLE h = CreateFileA(src.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    WriteFile(h, "src content", 11, &written, NULL);
    CloseHandle(h);

    h = CreateFileA(dst.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    WriteFile(h, "dst content", 11, &written, NULL);
    CloseHandle(h);

    // destination exists: bFailIfExists=TRUE must fail and keep dst intact
    EXPECT_FALSE(CopyFileA(src.c_str(), dst.c_str(), TRUE));
    HANDLE hr = CreateFileA(dst.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(hr != INVALID_HANDLE_VALUE);
    char buf[16] = {0};
    DWORD readed = 0;
    ReadFile(hr, buf, 11, &readed, NULL);
    CloseHandle(hr);
    EXPECT_STREQ(buf, "dst content");

    // bFailIfExists=FALSE overwrites
    EXPECT_TRUE(CopyFileA(src.c_str(), dst.c_str(), FALSE));
    hr = CreateFileA(dst.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(hr != INVALID_HANDLE_VALUE);
    memset(buf, 0, sizeof(buf));
    ReadFile(hr, buf, 11, &readed, NULL);
    CloseHandle(hr);
    EXPECT_STREQ(buf, "src content");

    // bFailIfExists=TRUE succeeds when the destination is absent
    DeleteFileA(dst.c_str());
    EXPECT_TRUE(CopyFileA(src.c_str(), dst.c_str(), TRUE));
}

// ------------------------------------------------------------------------
// File enumeration (FindFirstFile/FindNextFile/FindClose)
// ------------------------------------------------------------------------

TEST(swinx_file, find_first_next_close)
{
    std::string dir = tempPath("find");
    ASSERT_TRUE(CreateDirectoryA(dir.c_str(), NULL));

    // two .txt files and one .log, so the "*.txt" filter must return exactly 2
    HANDLE h1 = CreateFileA((dir + "/one.txt").c_str(), GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, 0, 0);
    HANDLE h2 = CreateFileA((dir + "/two.txt").c_str(), GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, 0, 0);
    HANDLE h3 = CreateFileA((dir + "/three.log").c_str(), GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h1 != INVALID_HANDLE_VALUE && h2 != INVALID_HANDLE_VALUE &&
                h3 != INVALID_HANDLE_VALUE);
    CloseHandle(h1);
    CloseHandle(h2);
    CloseHandle(h3);

    WIN32_FIND_DATAA fd = {};
    HANDLE hFind = FindFirstFileA((dir + "/*.txt").c_str(), &fd);
    ASSERT_TRUE(hFind != INVALID_HANDLE_VALUE);

    std::string names[4];
    int count = 0;
    bool first = true;
    while (true)
    {
        ASSERT_LT(count, 4);
        names[count++] = fd.cFileName;
        // the wildcard "*.txt" must not match the "." / ".." pseudo entries
        EXPECT_STRNE(fd.cFileName, ".");
        EXPECT_STRNE(fd.cFileName, "..");
        if (!FindNextFileA(hFind, &fd))
            break;
        first = false;
    }
    EXPECT_FALSE(first);
    EXPECT_EQ(count, 2);
    EXPECT_TRUE(FindClose(hFind));

    for (int i = 0; i < count; i++)
    {
        std::string full = dir + "/" + names[i];
        EXPECT_NE(GetFileAttributesA(full.c_str()), INVALID_FILE_ATTRIBUTES);
        size_t dot = names[i].find(".txt");
        EXPECT_NE(dot, std::string::npos);
        EXPECT_EQ(dot + 4, names[i].size());
    }

    // cleanup (best effort)
    DeleteFileA((dir + "/one.txt").c_str());
    DeleteFileA((dir + "/two.txt").c_str());
    DeleteFileA((dir + "/three.log").c_str());
    RemoveDirectoryA(dir.c_str());
}

TEST(swinx_file, find_no_match_returns_invalid_handle)
{
    std::string dir = tempPath("find");
    ASSERT_TRUE(CreateDirectoryA(dir.c_str(), NULL));

    WIN32_FIND_DATAA fd = {};
    HANDLE hFind = FindFirstFileA((dir + "/*.zzz").c_str(), &fd);
    EXPECT_TRUE(hFind == INVALID_HANDLE_VALUE);

    RemoveDirectoryA(dir.c_str());
}

// ------------------------------------------------------------------------
// File attributes
// ------------------------------------------------------------------------

TEST(swinx_file, file_attributes_readonly_roundtrip)
{
    std::string path = tempPath("attr.bin");
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    CloseHandle(h);

    // a freshly created regular file is visible (exact bits differ:
    // real Windows reports ARCHIVE, swinx may report NORMAL)
    EXPECT_NE(GetFileAttributesA(path.c_str()), INVALID_FILE_ATTRIBUTES);

    // setting READONLY must clear the write permission
    ASSERT_TRUE(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_READONLY));
    DWORD attr = GetFileAttributesA(path.c_str());
    EXPECT_NE(attr, INVALID_FILE_ATTRIBUTES);
    EXPECT_TRUE(attr & FILE_ATTRIBUTE_READONLY);

    // writing through a new handle must fail while the file is read-only
    HANDLE hW = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL,
                            OPEN_EXISTING, 0, 0);
    EXPECT_TRUE(hW == INVALID_HANDLE_VALUE);

    // clear the attribute so the file can be removed again
    EXPECT_TRUE(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL));
    EXPECT_TRUE(DeleteFileA(path.c_str()));
    EXPECT_EQ(GetFileAttributesA(path.c_str()), INVALID_FILE_ATTRIBUTES);
}

// ------------------------------------------------------------------------
// Temp path / temp file name
// ------------------------------------------------------------------------

TEST(swinx_file, get_temp_path_and_temp_file_name)
{
    char tmp[MAX_PATH] = {0};
    DWORD n = GetTempPathA(MAX_PATH, tmp);
    EXPECT_GT(n, 0u);
    EXPECT_LT(n, (DWORD)MAX_PATH);
    // Win32 guarantees the returned path ends with a separator
    char last = tmp[n - 1];
    EXPECT_TRUE(last == '/' || last == '\\');
    // the temp directory must exist
    EXPECT_TRUE(GetFileAttributesA(tmp) & FILE_ATTRIBUTE_DIRECTORY);

    // GetTempFileNameA creates a real file whose name starts with the
    // (up to 3 chars of the) given prefix
    char name[MAX_PATH] = {0};
    UINT unique = GetTempFileNameA(tmp, "swx", 0, name);
    EXPECT_NE(unique, 0u);
    EXPECT_NE(GetFileAttributesA(name), INVALID_FILE_ATTRIBUTES);
    std::string base = name;
    size_t slash = base.find_last_of("/\\");
    ASSERT_NE(slash, std::string::npos);
    EXPECT_EQ(base.compare(slash + 1, 3, "swx"), 0);
    ASSERT_TRUE(base.size() > 4 && base.compare(base.size() - 4, 4, ".tmp") == 0);
    EXPECT_TRUE(DeleteFileA(name));
}

// ------------------------------------------------------------------------
// File size / SetEndOfFile
// ------------------------------------------------------------------------

TEST(swinx_file, set_end_of_file_truncates)
{
    std::string path = tempPath("trunc.bin");
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

    char buf[100];
    memset(buf, 'A', sizeof(buf));
    DWORD written = 0;
    ASSERT_TRUE(WriteFile(h, buf, sizeof(buf), &written, NULL));
    ASSERT_EQ(written, 100u);

    // move to offset 40 and truncate there
    DWORD pos = SetFilePointer(h, 40, NULL, FILE_BEGIN);
    ASSERT_EQ(pos, 40u);
    EXPECT_TRUE(SetEndOfFile(h));

    LARGE_INTEGER li = {0};
    EXPECT_TRUE(GetFileSizeEx(h, &li));
    EXPECT_EQ(li.QuadPart, 40LL);
    EXPECT_EQ(GetFileSize(h, NULL), 40u);

    CloseHandle(h);
    DeleteFileA(path.c_str());
}

TEST(swinx_file, get_file_size_ex_reports_size)
{
    std::string path = tempPath("size.bin");
    HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

    char buf[257];
    memset(buf, 'B', sizeof(buf));
    DWORD written = 0;
    ASSERT_TRUE(WriteFile(h, buf, sizeof(buf), &written, NULL));
    ASSERT_EQ(written, 257u);
    CloseHandle(h);

    // reopen read-only and query the size
    h = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    LARGE_INTEGER li = {0};
    EXPECT_TRUE(GetFileSizeEx(h, &li));
    EXPECT_EQ(li.QuadPart, 257LL);
    DWORD high = 0;
    EXPECT_EQ(GetFileSize(h, &high), 257u);
    EXPECT_EQ(high, 0u);
    CloseHandle(h);

    DeleteFileA(path.c_str());
}

// ------------------------------------------------------------------------
// Current directory
// ------------------------------------------------------------------------

TEST(swinx_file, current_directory_get_set)
{
    char saved[1024] = {0};
    ASSERT_TRUE(GetCurrentDirectoryA(sizeof(saved), saved) > 0);

    // chdir into a fresh temp dir and verify GetCurrentDirectoryA follows
    std::string dir = tempPath("cwd");
    ASSERT_TRUE(CreateDirectoryA(dir.c_str(), NULL));
    ASSERT_TRUE(SetCurrentDirectoryA(dir.c_str()));

    char now[1024] = {0};
    ASSERT_TRUE(GetCurrentDirectoryA(sizeof(now), now) > 0);
    // both sides return the path without a trailing separator
    std::string got = now;
    if (!got.empty() && (got[got.size() - 1] == '/' || got[got.size() - 1] == '\\'))
        got.erase(got.size() - 1);
    EXPECT_TRUE(got == dir || got == std::string("/private")+dir);

    // restore
    ASSERT_TRUE(SetCurrentDirectoryA(saved));
    EXPECT_TRUE(RemoveDirectoryA(dir.c_str()));
}

// ------------------------------------------------------------------------
// File times
// ------------------------------------------------------------------------

TEST(swinx_file, set_get_file_time_roundtrip)
{
    std::string path = tempPath("ftime.bin");
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

    // a fixed timestamp: 100ns ticks since 1601-01-01 (about 2022-12-18)
    FILETIME ft;
    ULARGE_INTEGER ul;
    ul.QuadPart = 133160352000000000ULL;
    ft.dwLowDateTime = ul.LowPart;
    ft.dwHighDateTime = ul.HighPart;

    // only the last-write time is asserted: it round-trips exactly on both
    // real NTFS (100ns granularity) and swinx (timespec, ns granularity)
    ASSERT_TRUE(SetFileTime(h, NULL, NULL, &ft));

    FILETIME ftWrite = {};
    ASSERT_TRUE(GetFileTime(h, NULL, NULL, &ftWrite));
    EXPECT_EQ(ftWrite.dwLowDateTime, ft.dwLowDateTime);
    EXPECT_EQ(ftWrite.dwHighDateTime, ft.dwHighDateTime);

    CloseHandle(h);
    DeleteFileA(path.c_str());
}

TEST(swinx_file, file_time_invalid_handle_fails)
{
    FILETIME ft = {};
    EXPECT_FALSE(SetFileTime(INVALID_HANDLE_VALUE, NULL, NULL, &ft));
    EXPECT_FALSE(GetFileTime(INVALID_HANDLE_VALUE, &ft, NULL, NULL));
}

// ------------------------------------------------------------------------
// Standard handles
// ------------------------------------------------------------------------

TEST(swinx_file, std_handle_write)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    ASSERT_TRUE(h != NULL && h != INVALID_HANDLE_VALUE);

    // writing a probe line to stdout must succeed on both platforms
    DWORD written = 0;
    EXPECT_TRUE(WriteFile(h, "std_handle_write\n", 17, &written, NULL));
    EXPECT_EQ(written, 17u);

    // invalid selector must not be handed back as a real handle
    EXPECT_TRUE(GetStdHandle((DWORD)-2) == INVALID_HANDLE_VALUE);
}

// ------------------------------------------------------------------------
// P1: MoveFile edge cases, wide-char enumeration, osfhandle roundtrip
// ------------------------------------------------------------------------

TEST_F(FileTest, move_file_existing_destination_fails)
{
    std::string src = dir + "/a.txt";
    std::string dst = dir + "/b.txt";

    HANDLE h = CreateFileA(src.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    WriteFile(h, "new content", 11, &written, NULL);
    CloseHandle(h);

    h = CreateFileA(dst.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    WriteFile(h, "old content", 11, &written, NULL);
    CloseHandle(h);

    // Win32 MoveFile does NOT replace an existing destination (that behaviour
    // belongs to MoveFileEx + MOVEFILE_REPLACE_EXISTING). Verified on real
    // Windows; swinx's POSIX-rename shortcut needed an explicit guard.
    EXPECT_FALSE(MoveFileA(src.c_str(), dst.c_str()));

    // moving a file onto itself is a successful no-op
    EXPECT_TRUE(MoveFileA(src.c_str(), src.c_str()));

    // source untouched, destination intact
    EXPECT_NE(GetFileAttributesA(src.c_str()), INVALID_FILE_ATTRIBUTES);
    HANDLE hr = CreateFileA(dst.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    ASSERT_TRUE(hr != INVALID_HANDLE_VALUE);
    char buf[16] = {0};
    DWORD readed = 0;
    ReadFile(hr, buf, 11, &readed, NULL);
    CloseHandle(hr);
    EXPECT_STREQ(buf, "old content");
}

TEST_F(FileTest, move_file_missing_source_fails)
{
    EXPECT_FALSE(MoveFileA((dir + "/no_such_file").c_str(), (dir + "/moved.txt").c_str()));
    // no destination must be created as a side effect
    EXPECT_EQ(GetFileAttributesA((dir + "/moved.txt").c_str()), INVALID_FILE_ATTRIBUTES);
}

TEST_F(FileTest, move_file_wide_char_names)
{
    // std::wstring src = tempPathW(L"move_w_src.txt");
    // std::wstring dst = tempPathW(L"move_w_dst.txt");
    // DeleteFileW(dst.c_str());

    // HANDLE h = CreateFileW(src.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    // ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    // DWORD written = 0;
    // WriteFile(h, "wide move", 9, &written, NULL);
    // CloseHandle(h);

    // EXPECT_TRUE(MoveFileW(src.c_str(), dst.c_str()));
    // EXPECT_EQ(GetFileAttributesW(src.c_str()), INVALID_FILE_ATTRIBUTES);
    // EXPECT_NE(GetFileAttributesW(dst.c_str()), INVALID_FILE_ATTRIBUTES);

    // // content survived the wide-char name conversion
    // HANDLE hr = CreateFileW(dst.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    // ASSERT_TRUE(hr != INVALID_HANDLE_VALUE);
    // char buf[16] = {0};
    // DWORD readed = 0;
    // ReadFile(hr, buf, 9, &readed, NULL);
    // CloseHandle(hr);
    // EXPECT_STREQ(buf, "wide move");

    // DeleteFileW(dst.c_str());
}

TEST(swinx_file, find_first_file_exw_wide_names)
{
    std::wstring dirw = tempPathW(L"wdir_wide");
    CreateDirectoryW(dirw.c_str(), NULL);

    const wchar_t *names[] = {L"宽一.txt", L"宽二.txt"};
    for (int i = 0; i < 2; i++)
    {
        std::wstring f = dirw + L"/" + names[i];
        HANDLE h = CreateFileW(f.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
        ASSERT_TRUE(h != INVALID_HANDLE_VALUE) << "file " << i;
        DWORD written = 0;
        WriteFile(h, "12345", 5, &written, NULL);
        CloseHandle(h);
    }

    WIN32_FIND_DATAW wfd;
    std::wstring pat = dirw + L"/*.txt";
    HANDLE h = FindFirstFileExW(pat.c_str(), FindExInfoStandard, &wfd,
                                FindExSearchNameMatch, NULL, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    int count = 0;
    bool saw1 = false, saw2 = false;
    do
    {
        if (wcscmp(wfd.cFileName, names[0]) == 0)
            saw1 = true;
        else if (wcscmp(wfd.cFileName, names[1]) == 0)
            saw2 = true;
        else
            continue;
        count++;
        EXPECT_EQ(wfd.nFileSizeLow, 5u);   // size survives the A/W bridge
    } while (FindNextFileW(h, &wfd));
    FindClose(h);

    EXPECT_EQ(count, 2);
    EXPECT_TRUE(saw1);
    EXPECT_TRUE(saw2);

    for (int i = 0; i < 2; i++)
        DeleteFileW((dirw + L"/" + names[i]).c_str());
    RemoveDirectoryW(dirw.c_str());
}

// CRT fd <-> win32 handle bridge. The two sides differ in handle identity
// (real Windows hands back the very same OS handle, swinx wraps the fd in a
// new syn handle), so assert on shared content/position, not pointer equality.
#ifdef _WIN32
#include <io.h>
#define FT_READ _read
#define FT_WRITE _write
#define FT_LSEEK _lseek
#define FT_CLOSE _close
#else
#include <unistd.h>
#define FT_READ read
#define FT_WRITE write
#define FT_LSEEK lseek
#define FT_CLOSE close
#endif

TEST_F(FileTest, osfhandle_roundtrip_shares_file_position)
{
    std::string path = dir + "/a.txt";
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);

#ifdef _WIN32
    // real Windows CRT: _open_osfhandle takes an intptr_t OS handle
    int fd = _open_osfhandle((intptr_t)h, 0);
    ASSERT_NE(fd, -1);

    // _get_osfhandle maps the fd back to the same OS handle
    EXPECT_TRUE(_get_osfhandle(fd) == (intptr_t)h);
#else
    int fd = _open_osfhandle(h, 0);
    ASSERT_NE(fd, -1);

    // swinx wraps the fd in a fresh syn handle: only validity is guaranteed
    HANDLE h2 = _get_osfhandle(fd);
    EXPECT_TRUE(h2 != NULL && h2 != INVALID_HANDLE_VALUE);
#endif

    // write through the win32 handle, read through the CRT fd
    DWORD written = 0;
    ASSERT_TRUE(WriteFile(h, "A", 1, &written, NULL));
    FT_LSEEK(fd, 0, SEEK_SET);
    char buf[2] = {0};
    ASSERT_EQ(FT_READ(fd, buf, 1), 1);
    EXPECT_EQ(buf[0], 'A');

    // write through the CRT fd, read back through the handle (position is shared)
    FT_LSEEK(fd, 0, SEEK_SET);
    ASSERT_EQ(FT_WRITE(fd, "B", 1), 1);
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    DWORD readed = 0;
    ASSERT_TRUE(ReadFile(h, buf, 1, &readed, NULL));
    EXPECT_EQ(buf[0], 'B');

#ifdef _WIN32
    // the CRT fd wraps the very same OS handle: closing it closes the handle
    ASSERT_EQ(FT_CLOSE(fd), 0);
#else
    // POSIX side: closing the handle closes the underlying fd
    CloseHandle(h);
#endif
}


// 迁自 test.cpp（原 file.createfile）：打开已存在的 uires.zip 并读取头部
TEST(swinx_file, open_existing_and_read)
{
    SOUI::SStringT srcDir = getSourceDir();
    SOUI::SStringT strZip = srcDir + _T("/uires.zip");
    HANDLE hFile = CreateFile(strZip.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    EXPECT_TRUE(hFile != INVALID_HANDLE_VALUE);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        char szBuf[100];
        DWORD readed = 0;
        BOOL bRead = ReadFile(hFile, szBuf, 100, &readed, 0);
        EXPECT_TRUE(bRead);
        CloseHandle(hFile);
    }
}
