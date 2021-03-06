//===-- ProcessLaunchInfo.h -------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_Target_FileAction_h
#define liblldb_Target_FileAction_h

#include <string>

namespace lldb_private
{

class FileAction
{
  public:
    enum Action
    {
        eFileActionNone,
        eFileActionClose,
        eFileActionDuplicate,
        eFileActionOpen
    };

    FileAction();

    void Clear();

    bool Close(int fd);

    bool Duplicate(int fd, int dup_fd);

    bool Open(int fd, const char *path, bool read, bool write);

    int
    GetFD() const
    {
        return m_fd;
    }

    Action
    GetAction() const
    {
        return m_action;
    }

    int
    GetActionArgument() const
    {
        return m_arg;
    }

    const char *GetPath() const;

    void
    Dump (Stream &stream) const;

  protected:
    Action m_action;    // The action for this file
    int m_fd;           // An existing file descriptor
    int m_arg;          // oflag for eFileActionOpen*, dup_fd for eFileActionDuplicate
    std::string m_path; // A file path to use for opening after fork or posix_spawn
};

} // namespace lldb_private

#endif
