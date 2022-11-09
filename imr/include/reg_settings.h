/******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software
 * and to discontinue the availability of this software. By using this software,
 * you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 ******************************************************************************/

/** @file
 * This file contains IMR register and bit definitions
 */

#ifndef IMR_REG_H_
#define IMR_REG_H_

/**
 * The size of the register area of a single IMR,
 * UIO driver maps 0x2000 bytes.
 */
#define IMR_REG_SIZE        0x00002000U    // LSXn Register Size

 /**
  * IMR has a pixel cache. The dimension (bytes) in X direction is
  */
#define PIXCACHE_X_BYTES    64
 /**
   * IMR has a pixel cache. The dimension (lines) in Y direction is
   */
#define PIXCACHE_Y_LINES    64


/**
 *  register address offset definition
 */

/**
 * Control Registers
 */
#define IMR_REG_OFFSET_CR        0x008U    //!< Control Register
#define IMR_REG_OFFSET_SR        0x00cU    //!< Status Register
#define IMR_REG_OFFSET_SRCR      0x010U    //!< Status Clear Register
#define IMR_REG_OFFSET_ICR       0x014U    //!< Interrupt Control Register
#define IMR_REG_OFFSET_IMR       0x018U    //!< Interrupt Mask Register
#define IMR_REG_OFFSET_DLPR      0x020U    //!< DL Status Register
#define IMR_REG_OFFSET_EDLR      0x028U    //!< Executed DL Status Register

/**
 *  Memory Control Registers
 */
#define IMR_REG_OFFSET_DLSAR    0x030U    //!< DL Start Address Register
#define IMR_REG_OFFSET_DSAR     0x034U    //!< Destination Start Address Register
#define IMR_REG_OFFSET_SSAR     0x038U    //!< Source Start Address Register
#define IMR_REG_OFFSET_DSTR     0x03cU    //!< Destination Stride Register
#define IMR_REG_OFFSET_SSTR     0x040U    //!< Source Stride Register
#define IMR_REG_OFFSET_DSOR     0x050U    //!< Destination Start Offset Address Register


/**
 *  Rendering Control Registers
 */
#define IMR_REG_OFFSET_CMRCR            0x054U    //!< Rendering Mode Register
#define IMR_REG_OFFSET_CMRCSR           0x058U    //!< Rendering Mode Set Register
#define IMR_REG_OFFSET_CMRCCR           0x05cU    //!< Rendering Mode Clear Register
#define IMR_REG_OFFSET_TRIMR            0x060U    //!< Triangle Mode Register
#define IMR_REG_OFFSET_TRIMSR           0x064U    //!< Triangle Mode Set Register
#define IMR_REG_OFFSET_TRIMCR           0x068U    //!< Triangle Mode Clear Register
#define IMR_REG_OFFSET_TRICR            0x06cU    //!< Triangle Color Register
#define IMR_REG_OFFSET_UVDPOR           0x070U    //!< Source and Destination Coordinate Decimal Point Register
#define IMR_REG_OFFSET_SUSR             0x074U    //!< Source Width Register
#define IMR_REG_OFFSET_SVSR             0x078U    //!< Source Height Register
#define IMR_REG_OFFSET_XMINR            0x080U    //!< MIN Clipping X Register
#define IMR_REG_OFFSET_YMINR            0x084U    //!< MIN Clipping Y Register
#define IMR_REG_OFFSET_XMAXR            0x088U    //!< MAX Clipping X Register
#define IMR_REG_OFFSET_YMAXR            0x08cU    //!< MAX Clipping Y Register
#define IMR_REG_OFFSET_AMXSR            0x090U    //!< Mesh Generation X Size Register
#define IMR_REG_OFFSET_AMYSR            0x094U    //!< Mesh Generation Y Size Register
#define IMR_REG_OFFSET_AMXOR            0x098U    //!< Mesh Generation X Start Register
#define IMR_REG_OFFSET_AMYOR            0x09cU    //!< Mesh Generation Y Start Register
#define IMR_REG_OFFSET_TRICR2           0x0a0U    //!< Triangle Color Register 2
#define IMR_REG_OFFSET_YLMINR           0x0b0U    //!< Minimum Luminance Correction Y Register
#define IMR_REG_OFFSET_UBMINR           0x0b4U    //!< Minimum Hue Correction U Register
#define IMR_REG_OFFSET_VRMINR           0x0b8U    //!< Minimum Hue Correction V Register
#define IMR_REG_OFFSET_YLMAXR           0x0bcU    //!< Maximum Luminance Correction Y Register
#define IMR_REG_OFFSET_UBMAXR           0x0c0U    //!< Maximum Hue Correction U Register
#define IMR_REG_OFFSET_VRMAXR           0x0c4U    //!< Maximum Hue Correction V Register

#define IMR_REG_OFFSET_CPDPOR           0x0d0U    //!< Correction Decimal Point Register
#define IMR_REG_OFFSET_YLCPR            0x0d4U    //!< Luminance correction Parameter Y register
#define IMR_REG_OFFSET_UBCPR            0x0d8U    //!< Hue correction Parameter U register
#define IMR_REG_OFFSET_VRCPR            0x0dcU    //!< Hue correction Parameter V register


#define IMR_REG_OFFSET_CMRCR2           0x0e4U    //!< Rendering Mode Register 2
#define IMR_REG_OFFSET_CMRCSR2          0x0e8U    //!< Rendering Mode Set Register 2
#define IMR_REG_OFFSET_CMRCCR2          0x0ecU    //!< Rendering Mode Clear Register 2


/**
 * V3U additions:
 */
#define IMR_REG_OFFSET_SSAOR           0x01A0U  //!< Source Start Address Offset Register
#define IMR_REG_OFFSET_UVSSAOR         0x01B4U  //!< Source Start Address Offset Register fot UV plane
#define IMR_REG_OFFSET_DSAOR           0x01C8U  //!< Destination Start Address Offset Register
#define IMR_REG_OFFSET_XYOFSR          0x01CCU  //!< X/Y Offset Register
#define IMR_REG_OFFSET_UVOFSR          0x01D0U  //!< u/v Offset Register


/**
 * Control Register bit definition
 */
#define IMR_REG_CR_SWRST        (0x1U << 15) //!< Software Reset
#define IMR_REG_CR_RS           (0x1U << 0) //!< Rendering Start

/**
 *  Status register bit definition
 */
#define IMR_REG_SR_REN            (0x1U << 5)    //!< Rendering in progress
#define IMR_REG_SR_INT            (0x1U << 2)    //!< INT Instruction Decode
#define IMR_REG_SR_IER            (0x1U << 1)    //!< Illegal Instruction Decode
#define IMR_REG_SR_TRA            (0x1U << 0)    //!< Trap

/**
 *  ICR (interrupt control register) bit definition
 */
#define IMR_REG_ICR_INTENB       (0x1U << 2)    //!< INT instruction interrupt
#define IMR_REG_ICR_IERENB       (0x1U << 1)    //!< illegal instruction interrupt
#define IMR_REG_ICR_TRAENB       (0x1U << 0)    //!< TRAP instruction interrupt

/**
 *  IMR (interrupt mask register) bit definition
 */
#define IMR_REG_IMR_BASE      (0x3U << 3)    //!< Reserved The write value should always be 1.
#define IMR_REG_IMR_INT       (0x1U << 2)    //!< INT Instruction Decode
#define IMR_REG_IMR_IER       (0x1U << 1)    //!< Illegal Instruction Decode
#define IMR_REG_IMR_TRA       (0x1U << 0)    //!< Trap

/**
 * CMRCR (control register) bit definition
 */
#define IMR_REG_CMRCR_CLSM    (0x1U << 19)    //!< Hue correction scale parameter specification
#define IMR_REG_CMRCR_CLOM    (0x1U << 18)    //!< Hue correction offset parameter specification
#define IMR_REG_CMRCR_LUSM    (0x1U << 17)    //!< Luminance correction scale parameter specification
#define IMR_REG_CMRCR_LUOM    (0x1U << 16)    //!< Luminance correction offset parameter specification
#define IMR_REG_CMRCR_CP16E   (0x1U << 15)    //!< Copy 16-bit mode
#define IMR_REG_CMRCR_YCM     (0x1U << 14)    //!< YC mode 0=processes Y 1=processes UV
#define IMR_REG_CMRCR_SY12    (0x1U << 12)    //!< Set this bit when Y data is input in 12-bpp precision.
#define IMR_REG_CMRCR_SY10    (0x1U << 11)    //!< Set this bit when Y data is input in 10-bpp precision.
#define IMR_REG_CMRCR_Y12     (0x1U << 9)     //!< Set this bit when Y data is output in 12-bpp precision.
#define IMR_REG_CMRCR_Y10     (0x1U << 8)     //!< Set this bit when Y data is output in 10-bpp precision.
#define IMR_REG_CMRCR_YISM    (0x1U << 7)     //!< Selects the output format for YUV data.
#define IMR_REG_CMRCR_SUV     (0x1U << 5)     //!< SUV[1:0] source color precision selection
#define IMR_REG_CMRCR_DUV     (0x1U << 3)     //!< DUV[1:0] destination color precision selection
#define IMR_REG_CMRCR_CLCE    (0x1U << 2)     //!< Hue correction enable
#define IMR_REG_CMRCR_LUCE    (0x1U << 1)     //!< Luminance correction enable

/**
 * CMRCR2 (control register 2) bit definition
 */
#define IMR_REG_CMRCR2_DCTE      (0x1U << 15)    //!< Tile address mode for destination enable
#define IMR_REG_CMRCR2_TCTE      (0x1U << 12)    //!< Tile address mode for texture (src) enable
#define IMR_REG_CMRCR2_UVFORM    (0x1U << 6)     //!< Swap order of adjacent U and V on YUV422 interleaved texture data
#define IMR_REG_CMRCR2_UV422FORM (0x1U << 5)     //!< Swap order of adjacent Y and UV on YUV422 interleaved texture data
#define IMR_REG_CMRCR2_YUV422E   (0x1U << 2)     //!< YUV422 interleave mode enable
#define IMR_REG_CMRCR2_LUTE      (0x1U << 0)     //!< Lookup table enable

/**
 *  TRIMR (triangle mode register) bit definition
 */
#define IMR_REG_TRIMR_BASE      (0)             //!< pre-V3x: The write value should always be bit11=1, V3M and later: TFE bit.
#define IMR_REG_TRIMR_TFE       (0x1U << 11)    //!< Trilinear filter mode enable
#define IMR_REG_TRIMR_TCM       (0x1U <<  6)    //!< Triangle Clockwise Mode
#define IMR_REG_TRIMR_AUTOSG    (0x1U <<  3)    //!< Automatic Source Coordinate Generation Mode
#define IMR_REG_TRIMR_AUTODG    (0x1U <<  2)    //!< Automatic Destination Coordinate Generation Mode
#define IMR_REG_TRIMR_BFE       (0x1U <<  1)    //!< Bilinear Filter Enable
#define IMR_REG_TRIMR_TME       (0x1U <<  0)    //!< Texture Mapping Enable

/**
 *  TRICR (triangle color register) control bit definition
 */
#define IMR_REG_TRICR_YCFORM   (0x1U << 31)    // for interleaved output set output format to UYVY or VYUY

/**
 *  IMR DL instruction definition
 */

/**
 * Display list opcode TRI
 */
#define IMR_OP_TRI(n)          ((0x8aU << 24) | ((n) & 0xFFFF))

/**
 * Display list opcode NOP
 */
#define IMR_OP_NOP(n)          ((0x80U << 24) | ((n) & 0xFFFF))

/**
 * Display list opcode TRAP
 */
#define IMR_OP_TRAP            (0x8fU << 24)

/**
 * Display list opcode WTL
 */
#define IMR_OP_WTL(add, n)     ((0x81U << 24) | (((add) / 4) << 16) | ((n) & 0xFFFF))

/**
 * Display list opcode WTL2
 */
#define IMR_OP_WTL2(add, n)    ((0x83U << 24) | (((add) / 4) << 10) | ((n) & 0x3FF))

/**
 * Display list opcode WTS
 */
#define IMR_OP_WTS(add, data)  ((0x82U << 24) | (((add) / 4) << 16) | ((data) & 0xFFFF))

/**
 * Display list opcode INT
 */
#define IMR_OP_INT             (0x88U << 24)

/**
 * Display list opcode SYNCM
 */
#define IMR_OP_SYNCM           (0x86U << 24)

/**
 * Display list opcode GOSUB
 */
#define IMR_OP_GOSUB           (0x8CU << 24)

/**
 * Display list opcode RET
 */
#define IMR_OP_RET             (0x8DU << 24)

#endif /* IMR_REG_H_ */
