/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <systemctrl.h>
#include <macros.h>
#include <prospu.h>
#include "sound/registers.h"

// Types used in both callbacks
#define SIGNED_BYTE 0
#define SIGNED_SHORT 1
#define SIGNED_INT 2

// Types used only in read callback
#define UNSIGNED_SHORT 5

// 2 Milliseconds PSX Cycle
#define SPU_UPDATE_INTERVAL 32

// SPU Status
int running = 0;

// SPU Background Thread
int spuThread(SceSize args, void * argp)
{
	// Set SPU Status
	running = 1;
	
	// Endless Loop
	while(running)
	{
		#ifdef WITH_PEOPS_SPU
		// Process Samples
		SPUasync(SPU_UPDATE_INTERVAL);
		#endif
		
		// Clear SPU Thread Busy Flag
		_sb(-2, PSP_SPU_REGISTER + 0x293);
		
		// Set left number of to-be-processed samples to 0 (done processing)
		_sh(0, PSP_SPU_REGISTER + 0x290);
		
		// Disable SPU in PSX SPU Status Register
		// (I don't know if this is clever... we shouldn't do that in the final SPU plugin...)
		// spuWriteCallback(0x1AA, spuReadCallback(0x1AA, SIGNED_SHORT) & 0x7FFF, SIGNED_SHORT);
		
		// Synchronize to 2 Milliseconds
		sceKernelDelayThread(2000);
	}
	
	#ifdef WITH_PEOPS_SPU
	// Close SPU
	SPUclose();
	
	// Shutdown SPU
	SPUshutdown();
	#endif
	
	// Destroy Background Thread
	sceKernelExitDeleteThread(0);
	
	// Shut up GCC
	return 0;
}

// SPU Background Thread Starter
void _sceMeAudio_DE630CD2(void * loopCore, void * stack)
{
	// Right now the Audio Thread doesn't work properly yet...
	// This is due to its nature of being optimized for Media Engine.
	// Especially the fact that it is using $ra for calculations are deadly
	// for the Main CPU... thus... a temporary "No-Sound-Workaround".
	// unsigned int text_addr = (unsigned int)loopCore;
	
	// This Patch fixes the Movie & CDDA Blocking Issue for now...
	// unsigned int waitForAudio = 0x83E8;
	// _sw(JR_RA, text_addr + waitForAudio);
	// _sw(LI_V0(0), text_addr + waitForAudio + 4);
	
	// Flush Cache
	// flushCache();
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	#ifdef WITH_PEOPS_SPU
	// Initialize PEOPS SPU Plugin
	SPUinit();
	
	// Open PEOPS SPU Plugin
	SPUopen();
	#endif
	
	// Spawn Background Thread
	sceKernelStartThread(sceKernelCreateThread("SPUThread", spuThread, 0x10, 32 * 1024, 0, NULL), 0, NULL);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
}

// IRQ Disabler
void removeIRQ(unsigned int * irqdata)
{
	// List Root Element
	unsigned int * root = (unsigned int *)(gp + 0x1B8);
	
	// Extract Pointer
	unsigned int * next = (unsigned int *)irqdata[0];
	unsigned int * previous = (unsigned int *)irqdata[1];
	
	// Unlink IRQ Data Item from List
	{
		// Next->Previous = Previous
		next[1] = (unsigned int)previous;
		
		// Previous->Next = Next
		previous[0] = (unsigned int)next;
	}
	
	// Previous Element is IRQ List Root
	if(previous == root)
	{
		// Some kind of counter maybe?
		if(_lw(gp + 0x1B0) > 0)
		{
			// Fetch Delta Basedata
			unsigned int fromnext = next[2];
			unsigned int fromglobal = _lw(gp + 0x1AC);
			
			// Override Global Clone
			_sw(fromnext, gp + 0x1AC);
			
			// Calculate and store Delta
			_sw(_lw(gp + 0x1B0) + fromnext - fromglobal, gp + 0x1B0);
		}
	}
	
	// Unlink Previous Item from current Item ? (why does it do that ?)
	irqdata[1] = 0;
}

// IRQ Enabler
void addIRQ(unsigned int * irqdata, unsigned int arg2)
{
	// List Root Element
	unsigned int * root = (unsigned int *)(gp + 0x1B8);
	
	// Calculate Root Info
	unsigned int rootinfo = _lw(gp + 0x1AC) - _lw(gp + 0x1B0) + arg2;
	
	// Start of IRQ Item List
	unsigned int * irq = (unsigned int *)root[0];
	
	// Last Item in List
	unsigned int * lastitem = root;
	
	// Move to last Item in List
	while(1)
	{
		// How could this ever happen?
		if(irq == root) break;
		
		// Magic? No idea.
		if((irq[2] - rootinfo) > 0) break;
		
		// Save Last Item
		lastitem = irq;
		
		// Move to next Item
		irq = (unsigned int *)irq[0];
	}
	
	// Set Last Element as Next Element
	irqdata[0] = (unsigned int)irq;
	
	// Set Element before Last Element as Previous Element
	irqdata[1] = (unsigned int)lastitem;
	
	// Set Element before Last Element's Next Element to new Item
	lastitem[0] = (unsigned int)irqdata;
	
	// Set Last Elements Previous Element to new Item
	irq[1] = (unsigned int)irqdata;
	
	// No idea... but hey its there in the original.
	if(_lw(gp + 0x1B0) > 0 && lastitem == root)
	{
		// So lets clone it for now.
		_sw(arg2, gp + 0x1B0);
		_sw(rootinfo, gp + 0x1AC);
	}
	
	// Nooo ideaaaa.
	irqdata[2] = rootinfo;
}

// SPU Register Memory Getter Callback
unsigned int spuReadCallback(int address, int type)
{
	// Reduce ??? Value by 10
	_sw(_lw(gp + 432) - 10, gp + 432);
	
	// 16bit Result
	unsigned short result = 0;
	
	// Read Register from SPU Emulator
	if(SPUreadRegister(address, &result))
	{
		// 16bit Read
		if((type & 0x3) == SIGNED_SHORT) return result;
		
		// 8bit Read
		if(type == SIGNED_BYTE) return result & 0xFF;
		
		// 32bit Result Part #2
		unsigned short result2nd = 0;
		
		// Read Register from SPU Emulator
		if(SPUreadRegister(address + 2, &result2nd))
		{
			// Mix Results
			return result | (result2nd << 16);
		}
	}
	
	// Relative Address
	unsigned int reladdress = address & 0x3FF;
	
	// Full Address
	unsigned int fulladdress = PSP_SPU_REGISTER + reladdress;
	
	// Special Handling for SPU status register
	if(reladdress == 0x1AE && (type & 0x3) == SIGNED_SHORT)
	{
		// Load SPU Status Register Value
		unsigned short psxstatus = _lh(fulladdress);
		
		// Load POPS Emulator-specific SPU Status Register Additions
		unsigned char popsstatus = _lb(gp + 842);
		
		// Mix Values together
		return ((((psxstatus & 0xF7F) & 0x20) >> 5) << 7) | (psxstatus & 0xF7F) | popsstatus;
	}
	
	// Signed Byte Type
	if(type == SIGNED_BYTE) return _lb(fulladdress);
	
	// Signed Short Type
	if(type == SIGNED_SHORT) return _lh(fulladdress);
	
	// Signed Int Type
	if(type == SIGNED_INT)
	{
		// Reduce ??? Value by 7
		_sw(_lw(gp + 432) - 7, gp + 432);
		
		// Return Signed Int Value
		return _lw(fulladdress);
	}
	
	// Unsigned Short Type
	if(type == UNSIGNED_SHORT) return _lh(fulladdress);
	
	// Unsigned Byte Type
	return _lb(fulladdress);
}


// SPU Register Memory Setter Callback
void spuWriteCallback(int address, int value, int type)
{
	// Relative Address (SPU registers range is only 0x200 bytes long)
	unsigned int reladdress = address & 0x1FF;
	
	// 16bit Value Masking (PSX registers are 16bit wide)
	unsigned short usvalue = value & 0xFFFF;
	
	// Special Non-16bit Write Handling
	if(type != SIGNED_SHORT)
	{
		// 32bit Write
		if(type != SIGNED_BYTE)
		{
			// Write first 16bit
			spuWriteCallback(reladdress, usvalue, SIGNED_SHORT);
			
			// Move Pointer for next 16bit
			reladdress += 2;
			
			// Use HI-16bit of Value
			usvalue = value >> 16;
		}
		
		// 8bit Write
		else
		{
			// Mask Single Byte
			usvalue = value & 0xFF;
			
			// Change Target (can this writer only write shorts? can't this glitch?)
			reladdress--;
		}
	}
	
	// Write's reduce this global field... by 1...?
	_sw(_lw(gp + 432) - 1, gp + 432);
	
	// SPU status register
	if(reladdress == 0x1AE)
	{
		// Forward to SPU Emulator
		SPUwriteRegister(address, usvalue);
		
		// SPU status register is read only in POPS
		return;
	}
	
	// Voice Data Area Range (8x2 Bytes per Voice)
	if(reladdress < 0x180)
	{
		// Don't ask me... 0xE however is the loop address... could have to do with that.
		if((((reladdress & 0xE) ^ 0x6) < 1) & (usvalue < 0x200))
		{
			// 0x202 Override
			usvalue = 0x202;
		}
	}
	
	// Write Value to Register
	_sh(usvalue, PSP_SPU_REGISTER + reladdress);
	
	// Forward to SPU Emulator
	SPUwriteRegister(address, usvalue);
	
	// After-Write Special Handling for Non-Voice Areas
	if(reladdress >= 0x180)
	{
		// Special Treatment for Sound buffer IRQ address
		if(reladdress == 0x1A4)
		{
			// Check if IRQ Enable Bit is set in SPU control register and value == IRQ target buffer
			if(((_lh(PSP_SPU_REGISTER + 0x1AA) >> 6) & 1) == 1 && ((_lw(gp + 844) >> 2) ^ usvalue) < 1)
			{
				// Disable SPU Status Register Bit Overrides
				_sb(0x40, gp + 842);
			}
		}
		
		// Non-IRQ addresses
		else
		{
			// zeile 65
			if(reladdress >= 0x18C)
			{
				// zeile 67
				if(reladdress >= 0x190)
				{
					// SPU data register for non-DMA transfer
					if(reladdress == 0x1A8)
					{
						// No Idea...
						if(_lb(gp + 843) < 0x20)
						{
							// Fetch Pointer
							unsigned short * pointer = (unsigned short *)(gp + (_lb(gp + 843) << 1));
							
							// Copy Value in there too...
							pointer[388] = usvalue;
							
							// Move Pointer forward by 2 Bytes for next Run
							_sb(_lb(gp + 843) + 1, gp + 843);
						}
					}
					
					// 0x190 ~ 0x1A8 (not including)
					// (not including) 0x1A8 ~ 0x1FF
					else
					{
						if(reladdress >= 0x19C)
						{
							// SPU control bitflag register
							if(reladdress == 0x1AA)
							{
								// Backup GP + 888 Value
								unsigned int gp888 = _lw(gp + 888);
								
								// Store Copy of Bitflag Value
								_sh(usvalue, gp + 840);
								
								// No idea...
								_sw(_lw(gp + 428) - _lw(gp + 432), gp + 888);
								
								// Really no idea...
								if(((_lw(gp + 428) - _lw(gp + 432)) - gp888) >= 0x300)
								{
									// For sample synchronization maybe... one sample playback time is 22?
									if(_lw(PSP_SPU_REGISTER + 0x294) == _lw(gp + 892))
									{
										// Wait a bit...
										sceKernelDelayThread(22);
									}
								}
								
								// Override Timer Variable?
								_sw(_lw(PSP_SPU_REGISTER + 0x294), gp + 892);
								
								// Enable SPU control bitflag isn't set (this means shut SPU down)
								if(((usvalue >> 15) & 1) == 0)
								{
									// Remove bitflags a turned off SPU wouldn't need and update register
									_sh(usvalue & 0x3F, PSP_SPU_REGISTER + 0x1AA);
								}
								
								// Non-DMA Write (write through data register)
								if(((usvalue >> 4) & 0x3) == 1)
								{
									// Get Copy Length
									unsigned char length = _lb(gp + 843);
									
									// Get Target Memory Address
									unsigned int target = _lw(gp + 844);
									
									// Get Source Memory Address
									unsigned int source = gp + 776;
									
									// Copy Loop
									while(length > 0)
									{
										// Reduce Length
										length--;
										
										// Some Special PSX Memory Address?
										if(target == (_lh(PSP_SPU_REGISTER + 0x1A4) << 2))
										{
											// Force SPU Status Bit #6 to 1 (overrides PSX register bit in read callback)
											_sb(0x40, gp + 842);
										}
										
										// Move Target Address (should this really be BEFORE copying data?)
										target = ((target + 1) & 0x3FFFF);
										
										// Copy 16bit of Data
										_sh(_lh(source), PSP_SPU_REGISTER + 704 + (target << 1));
										
										// Move Source Pointer
										source += 2;
									}
									
									// Copy Current Target Pointer Position back into Memory
									_sw(target, gp + 844);
									
									// Erase Copy Length
									_sb(0, gp + 843);
								}
								
								// Disable IRQ (enable bit isn't set)
								if(((usvalue >> 6) & 1) == 0)
								{
									// IRQ is currently enabled
									if(_lw(gp + 852) != 0)
									{
										// IRQ disable callback
										removeIRQ((unsigned int *)(gp + 848));
									}
									
									// And some more stuff I don't understand...
									_sb(1, PSP_SPU_REGISTER + 0x292);
									
									// Remove SPU Register Status Bit Overrides
									_sb(0, gp + 842);
								}
								
								// Enable IRQ (enable bit is set)
								else
								{
									// IRQ is currently disabled
									if(_lw(gp + 852) == 0)
									{
										// IRQ enable callback
										addIRQ((unsigned int *)(gp + 848), 0x869);
									}
								}
							}
							
							else
							{
								// Sound buffer address setter
								if(reladdress == 0x1A6)
								{
									// Fetch IRQ Sound Buffer Value (for comparison with normal sound buffer)
									unsigned short irqsoundbuffer = _lh(PSP_SPU_REGISTER + 0x1A4);
									
									// Write Value
									_sw(usvalue << 2, gp + 844);
									
									// IRQ enabled and buffer == irqsoundbuffer
									if(((_lh(PSP_SPU_REGISTER + 0x1AA) >> 6) & 1) == 1 && (usvalue ^ irqsoundbuffer) < 1)
									{
										// Force SPU Status Bit #6 to 1 (overrides PSX register bit in read callback)
										_sb(0x40, gp + 842);
									}
								}
								
								else
								{
									// Reverb work area start register
									// It seems unknown registers are handled as reverb work start too...
									if(reladdress == 0x1A2 || reladdress < 512)
									{
										// Don't ask me..
										_sb(1, PSP_SPU_REGISTER + 0x29D);
										_sb(1, PSP_SPU_REGISTER + 0x29F);
										
										// Wait...
										while(_lh(PSP_SPU_REGISTER + 0x29E) == 0x101)
										{
											// Do nothing...
										}
										
										// I really don't know...
										_sw(_lw(PSP_SPU_REGISTER + 0x288) | 0x80000000, PSP_SPU_REGISTER + 0x288);
										_sb(0, PSP_SPU_REGISTER + 0x29D);
									}
								}
							}
						}
						
						else
						{
							// Backup GP + 888 Value
							unsigned int gp888 = _lw(gp + 888);
							
							// Update GP + 888 Value
							_sw(_lw(gp + 428) - _lw(gp + 432), gp + 888);
							
							// No clue~
							if((_lw(gp + 428) - _lw(gp + 432) - gp888) >= 0x300)
							{
								// Again this weird wait comparison...
								if(_lw(PSP_SPU_REGISTER + 0x294) == _lw(gp + 892))
								{
									// Alright we wait...
									sceKernelDelayThread(22);
								}
							}
							
							// Overwrite Wait Timer
							_sw(_lw(PSP_SPU_REGISTER + 0x294), gp + 892);
						}
					}
				}
				
				else
				{
					// Valid Value
					if(usvalue != 0)
					{
						// Backup GP + 888 Value
						unsigned int gp888 = _lw(gp + 888);
						
						// Update GP + 888 Value
						_sw(_lw(gp + 428) - _lw(gp + 432), gp + 888);
						
						// No clue~
						if((_lw(gp + 428) - _lw(gp + 432) - gp888) >= 0x300)
						{
							// Again this weird wait comparison...
							if(_lw(PSP_SPU_REGISTER + 0x294) == _lw(gp + 892))
							{
								// Alright we wait...
								sceKernelDelayThread(22);
							}
						}
						
						// Overwrite Wait Timer
						_sw(_lw(PSP_SPU_REGISTER + 0x294), gp + 892);
						
						// Don't ask me..
						_sb(1, PSP_SPU_REGISTER + 0x29D);
						_sb(1, PSP_SPU_REGISTER + 0x29F);
						
						// Wait...
						while(_lh(PSP_SPU_REGISTER + 0x29E) == 0x101)
						{
							// Do nothing...
						}
						
						// I really don't know...
						_sw(_lw(PSP_SPU_REGISTER + 0x284) | (usvalue << ((reladdress << 3) - 3168)), PSP_SPU_REGISTER + 0x284);
						_sb(0, PSP_SPU_REGISTER + 0x29D);
					}
				}
			}
			
			else
			{
				// Valid Value
				if(usvalue != 0)
				{
					// Backup GP + 888 Value
					unsigned int gp888 = _lw(gp + 888);
					
					// Update GP + 888 Value
					_sw(_lw(gp + 428) - _lw(gp + 432), gp + 888);
					
					// No clue~
					if((_lw(gp + 428) - _lw(gp + 432) - gp888) >= 0x300)
					{
						// Again this weird wait comparison...
						if(_lw(PSP_SPU_REGISTER + 0x294) == _lw(gp + 892))
						{
							// Alright we wait...
							sceKernelDelayThread(22);
						}
					}
					
					// Overwrite Wait Timer
					_sw(_lw(PSP_SPU_REGISTER + 0x294), gp + 892);
					
					// Don't ask me..
					_sb(1, PSP_SPU_REGISTER + 0x29D);
					_sb(1, PSP_SPU_REGISTER + 0x29F);
					
					// Wait...
					while(_lh(PSP_SPU_REGISTER + 0x29E) == 0x101)
					{
						// Do nothing...
					}
					
					// Temporary Variable
					unsigned int temp = usvalue << ((reladdress << 3) - 3163);
					
					// Update Register (??) 0x280
					_sw(_lw(PSP_SPU_REGISTER + 0x280) | temp, PSP_SPU_REGISTER + 0x280);
					
					// Update Register (??) 0x284
					_sw(_lw(PSP_SPU_REGISTER + 0x284) & (!temp), PSP_SPU_REGISTER + 0x284);
					
					// And reset to zero again... could be a thread semaphore?
					_sb(0, PSP_SPU_REGISTER + 0x29D);
				}
			}
		}
	}
	
	// After-Write Special Handling for Voice Data Addresses (< 0x180)
	else
	{
		// Backup GP + 888 Value
		unsigned int gp888 = _lw(gp + 888);
		
		// Update GP + 888 Value
		_sw(_lw(gp + 428) - _lw(gp + 432), gp + 888);
		
		// No clue~
		if((_lw(gp + 428) - _lw(gp + 432) - gp888) >= 0x300)
		{
			// Again this weird wait comparison...
			if(_lw(PSP_SPU_REGISTER + 0x294) == _lw(gp + 892))
			{
				// Alright we wait...
				sceKernelDelayThread(22);
			}
		}
		
		// Overwrite Wait Timer
		_sw(_lw(PSP_SPU_REGISTER + 0x294), gp + 892);
		
		// Don't ask me..
		_sb(1, PSP_SPU_REGISTER + 0x29D);
		_sb(1, PSP_SPU_REGISTER + 0x29F);
		
		// Wait...
		while(_lh(PSP_SPU_REGISTER + 0x29E) == 0x101)
		{
			// Do nothing...
		}
		
		// Temporary Variable
		unsigned int temp = 1 << (reladdress >> 4);
		
		// Update Register (??) 0x288
		_sw(_lw(PSP_SPU_REGISTER + 0x288) | temp, PSP_SPU_REGISTER + 0x288);
		
		// Writing Voice Repeat Address
		if((reladdress & 0xF) == 0xE)
		{
			// Update ... something related to repeat address?
			_sw(_lw(PSP_SPU_REGISTER + 0x2A0) | temp, PSP_SPU_REGISTER + 0x2A0);
		}
		
		// Okay, this has to be a "busy writing" thing! It happens too often!
		_sb(0, PSP_SPU_REGISTER + 0x29D);
	}
}

// Shutdown SPU
void spuShutdown(void)
{
	// Set Shutdown Flag
	running = 0;
	
	// Wait a second (doesn't hurt us)
	sceKernelDelayThread(1000000);
}

