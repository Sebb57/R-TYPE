# Player guide

## Start a local game

1. Build the project using the [README instructions](../README.md).
2. From the repository root, start the server:

   ```sh
   ./build/bin/r-type_server
   ```

3. In another terminal, start a client:

   ```sh
   ./build/bin/r-type_client 127.0.0.1 Player1
   ```

   The address and name are optional. The client defaults to `127.0.0.1` and `Player`.
4. Start another client for each additional player. On another computer, use the server's IPv4 address and allow TCP port `6969` and UDP port `6767` through the network firewall.

Keep the working directory at the repository root so the client can find its sprites, sounds, and font.

## Join and launch a room

Connect to a server, then select an existing lobby or create a room. Players can only join while a room is in the lobby. Each player must select **Mark ready**; wait until each is marked ready and shows as connected. The host then selects **Start game**. A game cannot be started until every room member is ready and has completed UDP linking.

## Controls

| Key | Action |
|---|---|
| Arrow keys | Move |
| Space | Fire |
| Enter | Activate the selected menu item |
| Escape | Go back, leave the room, or leave the game |

## During and after a game

The game runs for at most three minutes. Defeating an enemy gives 100 points to the player credited with the kill. A destroyed player sees the live scoreboard and can press Escape to leave or wait for the game to end. At game over, the final scores are shown; press Escape to quit the match.

## Troubleshooting

- **The client cannot connect:** confirm the server is running and that you entered its IPv4 address. The default TCP port is `6969`.
- **The room stays in linking state:** make sure UDP port `6767` is reachable between client and server.
- **Sprites, font, or sounds are missing:** launch the client from the repository root so `assets/` relative paths resolve.
- **The host cannot start:** every player must be ready and show as connected in the room.
