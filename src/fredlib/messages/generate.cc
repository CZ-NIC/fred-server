#include "src/fredlib/messages/generate.h"

namespace Fred {
namespace Messages {

template < CommChannel::Value COMM_CHANNEL >
void Generate::Into< COMM_CHANNEL >::exec(OperationContext &_ctx)
{
}

template void Generate::Into< CommChannel::SMS >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::EMAIL >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::LETTER >::exec(OperationContext &_ctx);

}//namespace Fred::Messages
}//namespace Fred
