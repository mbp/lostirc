#ifndef IRC_DEFINES_H
#define IRC_DEFINES_H

namespace IRC
{
    enum UserMode {
        OP = 1 << 1,
        HALFOP = 1 << 2,
        VOICE = 1 << 3,
        ADMIN = 1 << 4,
        OWNER = 1 << 5,
        NONE = 1 << 6
    };
}
#endif
