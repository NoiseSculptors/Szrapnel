#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/toolchain.h>
#include <string.h>

// #include "sine.h"

/* Grab the I2S controller via your overlay alias */
#define I2S_DEV_NODE DT_ALIAS(i2s_tx)

#define SAMPLE_FREQUENCY 48000 
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE 1
#define NUMBER_OF_CHANNELS (1U)

/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS      1
#define CONFIG_EXTRA_BLOCKS 1
#define TIMEOUT           (2000U)

#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (INITIAL_BLOCKS + CONFIG_EXTRA_BLOCKS)

K_MEM_SLAB_DEFINE_IN_SECT_STATIC(mem_slab, __nocache, BLOCK_SIZE, BLOCK_COUNT, 2);

int main(void)
{
	const struct device *const i2s_dev = DEVICE_DT_GET(I2S_DEV_NODE);
	struct i2s_config config;
	int ret = 0;

	/* Check if the I2S peripheral driver initialized successfully */
	if (!device_is_ready(i2s_dev)) {
		printk("%s device is not ready\n", i2s_dev->name);
		return 0;
	}

	/** Number of bits representing one data word. */
	config.word_size = SAMPLE_BIT_WIDTH;

	/** Number of words per frame. */
	config.channels = NUMBER_OF_CHANNELS;

	/** Data stream format as defined by I2S_FMT_* constants. */
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	
	/* The STM32 acts as Master (Controller) generating BCLK and WS */
	config.options = I2S_OPT_BIT_CLK_CONTROLLER | I2S_OPT_FRAME_CLK_CONTROLLER;
	
	/** Frame clock (WS) frequency, this is sampling rate. */
	config.frame_clk_freq = SAMPLE_FREQUENCY;

	/** Memory slab to store RX/TX data. */
	config.mem_slab = &mem_slab;

	/** Size of one RX/TX memory block (buffer) in bytes. */
	config.block_size = BLOCK_SIZE;


	/** Read/Write timeout. Number of milliseconds to wait in case TX queue
	 * is full or RX queue is empty, or 0, or SYS_FOREVER_MS.  */
	config.timeout = TIMEOUT;

	/* Configure the transmit (TX) channel registers */
	i2s_configure(i2s_dev, I2S_DIR_TX, &config);

    /* kick-start the DMA stream */
	i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_START);

	while (1) {
		void *mem_block;
		uint32_t block_size = BLOCK_SIZE;

        for(uint32_t i=0; i<BLOCK_SIZE; i++)
            (uint32_t*)(mem_block + i) = i&1;

		/* Write into the Zephyr I2S ring buffer queue */
		ret = i2s_buf_write(i2s_dev, mem_block, block_size);
		if (ret < 0) {
			printk("Failed to queue I2S buffer: %d\n", ret);
			break;
		}

		/* Give the system background processing thread breathing room */
		k_msleep(10);
	}

	/* Safety cleanup if the loop breaks unexpectedly */
	i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	printk("Stream halted.\n");
	return 0;
}
