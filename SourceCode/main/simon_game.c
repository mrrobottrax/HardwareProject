#include "simon_game.h"

#include <freertos/FreeRTOS.h>

#include "display.h"
#include "audio.h"
#include "sounds.h"
#include "input.h"
#include "meta_logic.h"

#define SEQ_LEN 10

static char num_to_char(int num)
{
    if (num >= 0 && num <= 9)
        return '0' + num;

    if (num == 11)
        return '*';

    if (num == 12)
        return '#';

    return '@';
}

void simon_game_task(void *pvParams)
{
    display_clear();
    display_set_dd_address(3);
    display_write_string("ENTER CODE");

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // 0-9 = numbers, 10 = *, 11 = #
    int sequence[SEQ_LEN] = {0};

    int current_sequence_number = 0;

    while (true)
    {
        // show numbers
        display_clear();

        int rand_num = rand() % 12; // basically even ;)

        sequence[current_sequence_number] = rand_num;
        ++current_sequence_number;

        printf("SEQ LEN %u\n", current_sequence_number);

        for (int i = 0; i < current_sequence_number; ++i)
        {
            audio_playfile(SOUND_FOLDER_SIMON, SOUND_SIMON_TONES_START + sequence[i]);

            printf("%c\n", num_to_char(sequence[i]));

            display_set_dd_address(7);
            display_write_data(num_to_char(sequence[i]));
            vTaskDelay(300 / portTICK_PERIOD_MS);

            // allow the player to see the first number for tutorialization
            if (current_sequence_number > 1)
            {
                display_set_dd_address(7);
                display_write_data(' ');
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }

        printf("WAITING FOR PLAYER\n");

        // wait for input of sequence
        input_data_t ignored_inputs = {0};
        for (int i = 0; i < current_sequence_number; ++i)
        {
            input_data_t input;
            while (true)
            {
                input = input_read();

                // ignore inputs until released once
                for (int i = 0; i < 12; ++i)
                {
                    if (input.keypad[i] == false)
                        ignored_inputs.keypad[i] = false;

                    if (ignored_inputs.keypad[i])
                        input.keypad[i] = false;
                }

                bool did_input = false;
                for (int i = 0; i < 12; ++i)
                {
                    if (!input.keypad[i])
                        continue;

                    ignored_inputs.keypad[i] = true;
                    did_input = true;
                }

                if (did_input)
                    break;

                vTaskDelay(1);
            }

            int num = 0;
            {
                int key = 0;
                for (int i = 0; i < 12; ++i)
                {
                    if (!input.keypad[i])
                        continue;

                    key = i;
                    break;
                }

                if (key >= 0 && key <= 8)
                {
                    num = key + 1;
                }
                else if (key == 9)
                {
                    num = 10;
                }
                else if (key == 10)
                {
                    num = 0;
                }
                else if (key == 11)
                {
                    num = 11;
                }
            }

            printf("%c\n", num_to_char(num));

            display_clear();
            display_set_dd_address(7);
            display_write_data(num_to_char(num));

            audio_playfile(SOUND_FOLDER_SIMON, SOUND_SIMON_TONES_START + num);

            if (num != sequence[i])
            {
                printf("INCORRECT\n");

                vTaskDelay(500 / portTICK_PERIOD_MS);
                audio_playfile(SOUND_FOLDER_SIMON, SOUND_SIMON_INCORRECT);
                vTaskDelay(300 / portTICK_PERIOD_MS);

                lose_game();
            }

            printf("CORRECT\n");
        }

        vTaskDelay(700 / portTICK_PERIOD_MS);
        display_clear();
        display_set_dd_address(4);
        display_write_string("CORRECT!");
        audio_playfile(SOUND_FOLDER_SIMON, SOUND_SIMON_CORRECT);
        vTaskDelay(700 / portTICK_PERIOD_MS);

        if (current_sequence_number >= SEQ_LEN)
        {
            win_game();
        }
    }
}