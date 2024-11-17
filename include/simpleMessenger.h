#pragma once

#include "messenger.h"
// useful includes for implementation
#include "server.h"
#include "client.h"

namespace SimpleMessenger {
    // run console based messenger with messenger supplied
    void simpleMessenger(Messenger& messenger);
}
