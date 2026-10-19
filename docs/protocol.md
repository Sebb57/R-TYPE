# Server communication protocol

The client uses TCP for connection and room management, then UDP for input, state snapshots, and gameplay events. All multi-byte integers use **little-endian** byte order. Strings are encoded as one unsigned byte of length followed by that many bytes. TCP strings are not null-terminated.

## TCP framing

Each TCP message is framed as:

| Field | Size | Meaning |
|---|---:|---|
| Length | 2 bytes | Message size, including the type byte; excludes this length field |
| Type | 1 byte | `server::Type` |
| Payload | variable | Type-specific data |

The maximum message length is 4096 bytes. `Connect` is the first message on a connection; its protocol version is currently `1`. Usernames must be 1–16 ASCII letters, digits, `_` or `-`. Room names may be up to 32 bytes.

## TCP messages

`server::Type` values are declared in `include/protocol/TcpProtocol.hpp`.

| Type | Direction | Payload |
|---|---|---|
| `Connect` (1) | Client → server | version `u8`, username string |
| `ConnectOk` (2) | Server → client | empty |
| `ListRooms` (3) | Client → server | empty |
| `RoomList` (4) | Server → client | count `u8`; repeated room records: id `u32`, name string, player count `u8`, capacity `u8`, state `u8` |
| `CreateRoom` (5) | Client → server | room name string, capacity `u8` |
| `JoinRoom` (6) | Client → server | room id `u32` |
| `RoomJoined` (7) | Server → client | room id `u32`, session token `u64`, slot `u8`, UDP port `u16` |
| `RoomUpdate` (8) | Server → room | host slot `u8`, player count `u8`; repeated player records: slot `u8`, username string, UDP-bound `u8`, ready `u8` |
| `LeaveRoom` (9) | Client → server | empty |
| `StartGame` (10) | Host → server | empty |
| `GameStarted` (11) | Server → room | empty |
| `Error` (12) | Server → client | error code `u8` |
| `SetReady` (13) | Client → server | ready `u8` (`0` or `1`) |

The TCP connection is stateful. A client authenticates first, then creates or joins a room. Joining a running room is rejected. The host can start only while the room is a lobby and every member is ready and UDP-bound. Error codes correspond to `server::LobbyError` in `include/server/common/ServerConfig.hpp`; malformed TCP messages may close the connection.

## UDP datagram header

Every UDP datagram starts with this 18-byte header:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 2 | Magic `0x6767` |
| 2 | 1 | Protocol version (`1`) |
| 3 | 1 | `UdpType` |
| 4 | 2 | Payload size, excluding header |
| 6 | 4 | Sequence number |
| 10 | 8 | Session token (`0` on server-originated datagrams) |

Datagrams larger than 1200 bytes are rejected. A client sends `Hello` with its session token to bind its UDP endpoint. The server replies with `HelloAck`; its 3-byte payload is player slot, server tick rate, and snapshot rate. The session token then associates that endpoint with the TCP-created room session.

## Gameplay UDP messages

| Type | Direction | Payload |
|---|---|---|
| `Hello` (1) | Client → server | empty; header carries the session token |
| `HelloAck` (2) | Server → client | slot `u8`, tick rate `u8`, snapshot rate `u8` |
| `Input` (3) | Client → server | held-key bitmask `u8`; Up `0x01`, Down `0x02`, Left `0x04`, Right `0x08`, Fire `0x10` |
| `Snapshot` (4) | Server → room | snapshot payload below; header sequence equals tick |
| `Event` (5) | Server → room | event payload below; header sequence equals event id |
| `Ping` (6) / `Pong` (7) | Client ↔ server | 4 opaque bytes echoed by the server |
| `Disconnect` (8) | Client → server | one payload byte; leaves the room |

Inputs are accepted only from the endpoint bound by `Hello`, and stale input sequence numbers are ignored.

### Snapshot payload

| Field | Size | Meaning |
|---|---:|---|
| Tick | 4 | Simulation tick |
| Seconds remaining | 2 | `0` at game over |
| Game over | 1 | `0` or `1` |
| Alive mask | 1 | Bit `n` means player slot `n` is alive |
| Scores | 16 | Four `u32` scores, slots 0–3 |
| Entity count | 1 | Maximum 100 in one snapshot |
| Entity records | 10 each | id `u32`, type `u8`, x `i16`, y `i16`, flags `u8` |

Entity ids on the wire are ECS ids plus one (`0` is avoided). Entity types are Unknown `0`, Player `1`, Projectile `2`, Enemy `3`, Bonus `4`. The low two flag bits identify a player slot. Bonus flag `0x04` marks an extra-projectile bonus. Coordinates are signed pixels.

### Event payload

| Field | Size | Meaning |
|---|---:|---|
| Event id | 4 | Monotonically increasing per game instance |
| Event type | 1 | `CollisionEffect` |
| Entity id | 4 | Wire entity id (ECS id plus one) |
| Extra | 4 | For current visual events: packed signed position, x in low 16 bits and y in high 16 bits |

Current event types are RedEnemy `1`, BluePlayer `2`, RedPlayer `3`, EnemyDestroyed `4`, and PlayerDestroyed `5`. Events are sent over UDP, so delivery is not guaranteed; clients ignore event ids they have already processed. State snapshots remain authoritative for entity presence and position.

## Source of truth

Packet enums and helpers: `include/protocol/`. TCP parsing and dispatch: `src/server/net/TcpConnection.cpp`. UDP parsing and packet output: `src/server/net/UdpServer.cpp`. The client decoder is in `src/client/network/NetworkClient.cpp`.
