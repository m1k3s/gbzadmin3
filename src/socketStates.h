#ifndef _socketStates_h_
#define _socketStates_h_

enum State {
    Okay = 0,
    SocketError,
    Rejected,
    BadVersion,
    CrippledVersion,
    Refused,
    ResolveFailure
};

#endif // _socketStates_h_
