//
// Created by vini84200 on 6/23/23.
//

#include "MessageComunication.h"

Message::Message(Request request) {
    buffer = request.SerializeAsString();
}
