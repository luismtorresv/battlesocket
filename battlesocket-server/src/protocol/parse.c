#include "protocol.h"

// Parse client message and return message type.
MessageType
parse_message (const char *message)
{
  if (strncmp (message, "SHOT", 4) == 0)
    return MSG_SHOT;
  if (strncmp (message, "SURRENDER", 9) == 0)
    return MSG_SURRENDER;
  return MSG_BAD_REQUEST;
}

// Parse and validate shot.
Shot
parse_shot (const char *message)
{
  Shot shot = { .is_valid_shot = false }; // Default value in case of failure.

  // Check correct format.
  char action[16] = { 0 };
  char pos_str[16] = { 0 };
  if (sscanf (message, "%15s %15s", action, pos_str) != 2)
    {
      return shot;
    }

  char row_char;
  int col_val;
  int consumed = 0; // This variable will keep the characters consumed.
                    // We use %n to obtain the number of characters read while
                    // parsing. If there are characters remaining after the
                    // `col_val`, it is a bad request.

  // Extract row and col.
  if (sscanf (pos_str, " %c%d%n", &row_char, &col_val, &consumed) != 2)
    {
      return shot;
    }

  // Validate domain of shot.
  if (row_char < 'A' || row_char > 'J' || col_val < 1 || col_val > BOARD_SIZE)
    {
      return shot;
    }

  shot.row = row_char - 'A';
  shot.col = col_val - 1;
  shot.is_valid_shot = true;

  return shot;
}
