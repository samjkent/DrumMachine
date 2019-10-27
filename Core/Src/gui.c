#include "gui.h"
#include "audio_channel.h"

extern int sequencer_channel;

void gui_task(void *p) {

  int previous_sequencer_channel = -1;
  while(1) {
    ILI9341_FillRectangle(10, 10, 100, 100, GUI_FOREGROUND_COLOUR);
    ILI9341_FillRectangle(10, 10, 100, 100, GUI_BACKGROUND_COLOUR);
  }

  while (1) {

    // Check if update is required
    if(previous_sequencer_channel != sequencer_channel) {
        // Set previous_sequencer_channel
        previous_sequencer_channel = sequencer_channel;

        // Update sample information
        char channel = sequencer_channel + '0';
        ILI9341_WriteString(5, 5, &channel, Font_11x18, GUI_FOREGROUND_COLOUR,
                            GUI_BACKGROUND_COLOUR);

        gui_draw_waveform(sequencer_channel, 0, 80);
    }

    ILI9341_WriteString(5, 140, (char *)"Sample", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(5, 150, (char *)"ADSR", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(5, 165, (char *)"Filter", Font_11x18,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(5, 190, (char *)"FX", Font_7x10, GUI_FOREGROUND_COLOUR,
                        GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(5, 190, (char *)"FX", Font_7x10, GUI_FOREGROUND_COLOUR,
                        GUI_BACKGROUND_COLOUR);

    ILI9341_DrawHLine(80, 140, 230, GUI_FOREGROUND_COLOUR);

    // Fake filter env
    for (int x = 0; x < 20; x++) {
      ILI9341_DrawPixel(100 + x, 180.0 - (2 * x), GUI_FOREGROUND_COLOUR);
    }
    for (int x = 0; x < 40; x++) {
      ILI9341_DrawPixel(120 + x, 140, GUI_FOREGROUND_COLOUR);
    }

    ILI9341_WriteString(170, 140, (char *)"Cut Off: 20 Hz", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(170, 150, (char *)"Resonance: 0%", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(170, 160, (char *)"LFO Amount: 20%", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(170, 170, (char *)"LFO Freq: 1/6", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);
    ILI9341_WriteString(170, 180, (char *)"Type: High Pass", Font_7x10,
                        GUI_FOREGROUND_COLOUR, GUI_BACKGROUND_COLOUR);

    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

void gui_draw_ticks(int fs, int div, int size, int windowSize, int x,
                    int yPos) {
  int next = ((x + 1) * windowSize) % (fs / div);
  int curr = ((x)*windowSize) % (fs / div);
  if (next < curr) {
    for (int16_t y = -size; y <= size; y++)
      ILI9341_DrawPixel(x, yPos + y, ILI9341_BLUE);
  }
}

void gui_draw_waveform(int track, int channel, int yPos) {

  // Clear waveform if changed
  ILI9341_FillRectangle(0, 40, ILI9341_WIDTH, 80, GUI_BACKGROUND_COLOUR);

  // For x 0->240
  int32_t memory;
  int16_t sample;
  int16_t max, min;
  int16_t prevSample = 0;
  int16_t windowSize = 100;
  uint32_t lastAddress = 0;
  // For x 0->240
  for (int x = 0; x < 300; x++) {
    max = 0;
    min = 0;

    for (int n = 0; n < windowSize; n++) {

      memory =
          *(int32_t *)(sequencer[sequencer_channel].sample_start +
                       (x * 4 * windowSize) +
                       (4 *
                        n)); // + (uint32_t)(x*windowSize) + (uint32_t)(32*n));

      if (channel == 0)
        sample = (int16_t)(memory >> 16);

      if (channel == 1)
        sample = (int16_t)(0xFFFF & memory);

      sample = sample / 640;

      if (sample > max)
        max = sample;
      if (sample < min)
        min = sample;
    }

    // If the window size is large the waveform will not be accurate
    // This provides a better overview of the window
    if (windowSize > 200) {
      for (int16_t y = min; y <= max; y++)
        ILI9341_DrawPixel(x + 10, yPos + y, ILI9341_YELLOW);
    } else {

      // Draw wave
      if (prevSample < sample) {
        for (int16_t y = prevSample; y <= sample; y++)
          ILI9341_DrawPixel(x + 10, yPos + y, ILI9341_GREEN);
      } else {
        for (int16_t y = prevSample; y >= sample; y--)
          ILI9341_DrawPixel(x + 10, yPos + y, ILI9341_GREEN);
      }
    }

    // Draw zero
    ILI9341_DrawPixel(x + 10, yPos, ILI9341_BLUE);

    // Draw .1 sec marker
    gui_draw_ticks(44100, 10, 2, windowSize, x, yPos);

    // Draw 1 sec marker
    gui_draw_ticks(44100, 1, 5, windowSize, x, yPos);

    prevSample = sample;
  }
}
