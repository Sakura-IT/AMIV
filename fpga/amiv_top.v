
/****************************************************************************
 *									SETUP									*
 *																			*
 *		AMIGA -> DB23 -> AD9984A -> FPGA -> AD7511W -> HDMI -> MONITOR		*
 *																			*
 ***************************************************************************/


module AMIV_TOP( input wire ref_clock,
				 input wire in_pclk,
				 input wire in_hs,
				 input wire in_vs,
				 input wire in_oddeven,
				 input wire [29:0] in_data,
				 output wire out_pclk,
				 output wire out_hs,
				 output wire out_vs,
				 output wire out_de,
				 output wire [23:0] out_data,
				 output wire sram_ce_n,
				 output wire sram_oe_n,
				 output wire sram_we_n,
				 output wire sram_lb_n,
				 output wire sram_ub_n,
				 output wire sram_busy_n,
				 output wire [18:0] sram_out_addr,
				 inout wire [15:0] sram_inout_data,
				 input wire gpio_0,
				 input wire gpio_1,
				 input wire gpio_2,
				 input wire gpio_3,
				 output wire out_iled,
				 input wire in_freq_select);

localparam IN_FRAME_PER_SECOND = 'd50; /* number of frames used by interlaced detection */
localparam IN_HORIZONTAL_BLANKING = 'd170;
localparam IN_VERTICAL_BLANKING = 'd30;

localparam IN_HORIZONTAL_BLANKING_OFFSET = 'd40;
localparam IN_VERTICAL_BLANKING_OFFSET = 'd15;

localparam OUT_HFP = 'd440; /* set to 110 for 720p @ 60Hz and to 440 for 720p @ 50Hz */
localparam OUT_HBP = 'd220;
localparam OUT_HSW = 'd40;
localparam OUT_VFP = 'd5;
localparam OUT_VBP = 'd20;
localparam OUT_VSW = 'd5;
localparam OUT_WIDTH = 'd1280;
localparam OUT_HEIGHT = 'd720;
localparam OUT_HORIZONTAL_BLANKING = OUT_HFP + OUT_HBP + OUT_HSW;
localparam OUT_VERTICAL_BLANKING = OUT_VFP + OUT_VBP + OUT_VSW;
localparam OUT_HORIZONTAL_PIXELS = OUT_WIDTH + OUT_HORIZONTAL_BLANKING;
localparam OUT_VERTICAL_LINES = OUT_HEIGHT + OUT_VERTICAL_BLANKING;
localparam OUT_WIDTH_OFFSET = 'd0;
localparam OUT_HEIGHT_OFFSET = 'd78; /* 720p got more lines than input format, in order to center picture, this is offset counted from the top of the screen */

localparam SRAM_WIDTH = 'd640;
localparam SRAM_HEIGHT = 'd580;

/* SRAM */
reg reg_sram_in_wr_n; /* goes low when a write should be performed */
reg reg_sram_in_wr_n_next;
reg reg_sram_oe_n; /* not needed for the sram used actually */
reg reg_sram_oe_n_next;
reg [18:0] reg_sram_in_addr;
reg [18:0] reg_sram_in_addr_next;
reg [15:0] reg_sram_in_data;
reg [15:0] reg_sram_in_data_next;
reg [18:0] reg_sram_write_pixel_counter; /* number of pixels written */
reg [18:0] reg_sram_write_pixel_counter_next;
reg reg_sram_flag_write;
reg reg_sram_flag_write_next;
reg [18:0] reg_sram_read_pixel_counter; /* number of pixels read */
reg [18:0] reg_sram_read_pixel_counter_next;
reg reg_sram_out_busy_n;

/* OUT */
reg reg_out_hs;
reg reg_out_hs_next;
reg reg_out_vs;
reg reg_out_vs_next;
reg reg_out_pclk;
reg reg_out_pclk_next;
reg reg_out_de; /* not used since the internal de generator in 7511 is used instead */
reg reg_out_de_next;
reg reg_out_line_render_twice_flag; /* this is used to know if a line has been rendered twice or not, not used in interlaced mode */
reg reg_out_line_render_twice_flag_next;
reg [23:0] reg_out_data;
reg [23:0] reg_out_data_next;
reg [11:0] reg_out_hs_pixel_counter;
reg [11:0] reg_out_hs_pixel_counter_next;
reg [9:0] reg_out_line_counter;
reg [9:0] reg_out_line_counter_next;
reg reg_out_iled;
wire out_clk;

/* IN */
reg [29:0] reg_in_data_saved;
reg [29:0] reg_in_data_saved_next;
reg [9:0] reg_in_hs_pixel_counter;
reg [9:0] reg_in_hs_pixel_counter_next;
reg [9:0] reg_in_line_counter;
reg [9:0] reg_in_line_counter_next;
reg [8:0] reg_in_horizontal_blanking;
reg [8:0] reg_in_horizontal_blanking_next;
reg [7:0] reg_in_vertical_blanking;
reg [7:0] reg_in_vertical_blanking_next;
reg [7:0] reg_interlaced_active_counter; /* used to detect interlaced mode */
reg [7:0] reg_interlaced_active_counter_next;
reg reg_interlaced_active; /* this is set if the mode is interlaced */
reg reg_interlaced_active_next;
reg reg_interlaced_detected; /* this will be set when odd/even field has switched from high to low during number of frames set in reg_interlaced_active_counter */
reg reg_interlaced_detected_next;

/* MISC */
reg [2:0] reg_state;
reg [2:0] state_next;
reg reg_reset_screen_pos = 0;
reg reg_output_active = 0;
reg reg_output_active_next = 0;

localparam [2:0] state_0 = 3'b000,
				 state_1 = 3'b001,
				 state_2 = 3'b010,
				 state_3 = 3'b011;


/* obtain an output clk of 148,5Mhz */
AMIV_PLL pll( .CLKI(ref_clock),
              .CLKOP(out_clk)); 

/******************************************************************/
/************************* INPUT **********************************/
/******************************************************************/

/*
 * When input clk in goes high, it means that a new pixel is ready
 * from ad9984a. Save the sram address where the pixel is intended to
 * written and the corresponding pixel data so the pixel can be written
 * to SRAM when it becomes available.
*/
always @ (posedge in_pclk)
begin
	reg_sram_write_pixel_counter <= reg_sram_write_pixel_counter_next;
	reg_in_hs_pixel_counter <= reg_in_hs_pixel_counter_next;
	reg_in_data_saved <= reg_in_data_saved_next;
end

always @*
begin
	/* default */
	reg_in_hs_pixel_counter_next = reg_in_hs_pixel_counter;
	reg_sram_write_pixel_counter_next = reg_sram_write_pixel_counter;
	reg_in_data_saved_next = reg_in_data_saved;
	
	/* if hs in is low, then pixel counter in should be set to zero */
	if(in_hs == 0) begin
		reg_in_hs_pixel_counter_next = 0;
	/* if hs in is high, then pixel counter in should be incremented */
	end else begin
		reg_in_hs_pixel_counter_next = reg_in_hs_pixel_counter + 1'b1;
	end

	/* if inside the window that should be rendered, then save the address where it should be stored on sram */
	if(reg_in_hs_pixel_counter >= reg_in_horizontal_blanking &&
			reg_in_hs_pixel_counter < (SRAM_WIDTH + reg_in_horizontal_blanking) &&
			reg_in_line_counter >= reg_in_vertical_blanking &&
			reg_in_line_counter < (SRAM_HEIGHT/2 + reg_in_vertical_blanking)) begin
		if(reg_interlaced_active == 1) begin
			/* even or odd line ? */
			if( in_oddeven == 0) begin
				reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
													(reg_in_line_counter - reg_in_vertical_blanking) * 2'd2 * SRAM_WIDTH;
			end else begin
				reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
													(reg_in_line_counter - reg_in_vertical_blanking) * 2'd2 * SRAM_WIDTH + SRAM_WIDTH;
			end
		end else begin
			reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
												(reg_in_line_counter - reg_in_vertical_blanking) * SRAM_WIDTH;
		end
	end else begin
		reg_sram_write_pixel_counter_next = 0;
	end

	/* store the data here, so that it can be written inside output statemachine */
	reg_in_data_saved_next = in_data;
end

/* 
 * When vs becomes high, the current frame is at its end
 * and a new frame will start. If odd-even indicator did not
 * change value within x number of vs, then mode is not interlaced.
 */
always @ (posedge in_vs)
begin
	reg_interlaced_active <= reg_interlaced_active_next;
	reg_interlaced_detected <= reg_interlaced_detected_next;
	reg_interlaced_active_counter <= reg_interlaced_active_counter_next;
end

always @*
begin
	if(reg_interlaced_active_counter > 0) begin
		reg_interlaced_active_counter_next = reg_interlaced_active_counter - 1'b1;

		if(in_oddeven == 0) begin
			reg_interlaced_detected_next = 1'b1;
		end

	end else begin
		reg_interlaced_active_counter_next = IN_FRAME_PER_SECOND;
		if(reg_interlaced_detected == 1'b1) begin
			reg_interlaced_active_next = 1'b1;
			reg_out_iled = 0;
		end else begin
			reg_interlaced_active_next = 0;
			reg_out_iled = 1;
		end
		
		reg_interlaced_detected_next = 0;
		
	end
end

/* 
 * When hs becomes high, the current line is at its end
 * and a new line will start. Use this to keeping track
 * of the number of lines displayed.
 */
always @ (posedge in_hs)
begin
	reg_in_line_counter <= reg_in_line_counter_next;
end

always @*
begin
	/* default */
	reg_in_line_counter_next = reg_in_line_counter;
	
	if(in_vs == 0) begin
		reg_in_line_counter_next = 0;
	end else begin
		reg_in_line_counter_next = reg_in_line_counter + 1'b1;
		
	end
end

/******************************************************************/
/************************* OUTPUT *********************************/
/******************************************************************/

/* 
 * The output clock is set as x2 freq of actual pixel clock.
 * In this statemachine, pixels will be read from sram at a
 * constant rate and output 2 times consecutively to adv7511.
 * Also, any input pixel that is waiting for service will be
 * written to sram.
 */
always @ (posedge out_clk)
begin
	if(reg_output_active == 1) begin
		reg_state <= state_next;
		reg_out_data <= reg_out_data_next;
		reg_out_pclk <= reg_out_pclk_next;
		reg_out_hs_pixel_counter <= reg_out_hs_pixel_counter_next;
		reg_out_line_render_twice_flag <= reg_out_line_render_twice_flag_next;
		reg_out_hs <= reg_out_hs_next;
		reg_out_line_counter <= reg_out_line_counter_next;
		reg_out_de <= reg_out_de_next;
		reg_out_vs <= reg_out_vs_next;
		reg_sram_read_pixel_counter <= reg_sram_read_pixel_counter_next;
		reg_sram_in_addr <= reg_sram_in_addr_next;
		reg_sram_in_data <= reg_sram_in_data_next;
		reg_sram_oe_n <= reg_sram_oe_n_next;
		reg_sram_in_wr_n <= reg_sram_in_wr_n_next;
		
		/* make sure that new pixel is written to sram only when it appropriate */
		if(in_pclk == 1'b1 && reg_sram_flag_write == 0) begin
			reg_sram_flag_write <= 1'b1;
		end else begin
			reg_sram_flag_write <= reg_sram_flag_write_next;
		end
	end
end

always @*
begin
	/* default */
	state_next = state_0;
	reg_out_data_next = reg_out_data;
	reg_out_pclk_next = reg_out_pclk;
	reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter;
	reg_out_line_render_twice_flag_next = reg_out_line_render_twice_flag;
	reg_out_line_counter_next = reg_out_line_counter;
	reg_out_hs_next = reg_out_hs;
	reg_out_de_next = reg_out_de;
	reg_out_vs_next = reg_out_vs;
	reg_sram_read_pixel_counter_next = reg_sram_read_pixel_counter;
	reg_sram_in_addr_next = reg_sram_in_addr;
	reg_sram_in_data_next = reg_sram_in_data;
	reg_sram_oe_n_next = 1'b0;
	reg_sram_in_wr_n_next = 1'b1;
	reg_sram_flag_write_next = reg_sram_flag_write;

	case(reg_state)
	
	state_0:
		begin
			state_next = state_1;
			
			/* time to clock out data read from sram */
			reg_out_pclk_next = 1'b1;
			reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter + 1'b1;
		end
	state_1:
		begin
			state_next = state_2;
			
			reg_out_pclk_next = 0;

			/* if we are inside the window that should be rendered */
			if(reg_out_hs_pixel_counter >= OUT_HORIZONTAL_BLANKING &&
			   reg_out_hs_pixel_counter < ((SRAM_WIDTH * 2) + OUT_HORIZONTAL_BLANKING) &&
			   reg_out_line_counter >= (OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET) &&
			   reg_out_line_counter < (SRAM_HEIGHT + OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET - (reg_in_vertical_blanking - IN_VERTICAL_BLANKING_OFFSET)*2)) begin
				
				/* put the pixel that was read from sram to output bus */
				reg_out_data_next[23:19] = sram_inout_data[15:11];
				reg_out_data_next[18:16] = 0;
				reg_out_data_next[15:10] = sram_inout_data[10:5];
				reg_out_data_next[9:8] = 0;
				reg_out_data_next[7:3] = sram_inout_data[4:0];
				reg_out_data_next[2:0] = 0;
			end else begin
				/* if not inside window just render black color */
				reg_out_data_next[23:19] = 0;
				reg_out_data_next[18:16] = 0;
				reg_out_data_next[15:10] = 0;
				reg_out_data_next[9:8] = 0;
				reg_out_data_next[7:3] = 0;
				reg_out_data_next[2:0] = 0;
			end
			
			/* pixel waiting to be written to sram? */
			if(reg_sram_flag_write == 1) begin
				reg_sram_flag_write_next = 0;
				reg_sram_in_wr_n_next = 1'b0;
				
				/* set the write address */
				reg_sram_in_addr_next = reg_sram_write_pixel_counter;
			end
		end
	state_2:
		begin
			state_next = state_3;
			reg_out_pclk_next = 1'b1;
			
			/* time to increase pixel counter out */
			reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter + 1;
			
			/* dummy section start (in order to configure the pins they must be "used") */
			if(reg_in_data_saved[24] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[23] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[22] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[21] == 1'b1)
				reg_sram_out_busy_n = 0;
			else if(reg_in_data_saved[20] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[13] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[12] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[11] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[10] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[4] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[3] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[2] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[1] == 1'b1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[0] == 1'b1)
				reg_sram_out_busy_n = 0;
			else if(in_oddeven == 1)
				reg_sram_out_busy_n = 0;
			
			/* dummy section end */
				
			/* put the data that should be written on sram bus */
			reg_sram_in_data_next[15:11] = reg_in_data_saved[29:25];
			reg_sram_in_data_next[10:5] = reg_in_data_saved[19:14];
			reg_sram_in_data_next[4:0] = reg_in_data_saved[9:5];
		end	
	state_3:
		begin
			state_next = state_0;
			
			/* time to clock out data read from sram */
			reg_out_pclk_next = 0;	
			
			/* if counters say that we are inside the window that should be rendered */
			if(reg_out_hs_pixel_counter >= OUT_HORIZONTAL_BLANKING &&
					reg_out_hs_pixel_counter < ((SRAM_WIDTH * 2) + OUT_HORIZONTAL_BLANKING) &&
					reg_out_line_counter >= (OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET) &&
					reg_out_line_counter < (SRAM_HEIGHT + OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET - (reg_in_vertical_blanking - IN_VERTICAL_BLANKING_OFFSET)*2)) begin
					
				/* increase read counter */
				reg_sram_read_pixel_counter_next = reg_sram_read_pixel_counter + 1'b1;
				
				/* set the read address */
				reg_sram_in_addr_next = reg_sram_read_pixel_counter;
			end else begin
				reg_sram_in_addr_next = 0;
			end
		end
	default:
		begin
			state_next = state_0;
		end
	endcase

	/* after OUT_HFP pixels, set hs out high for a duration of OUT_HSW (according to 720p timing spec) */
	if(reg_out_hs_pixel_counter >= OUT_HFP && reg_out_hs_pixel_counter < (OUT_HFP + OUT_HSW)) begin
		reg_out_hs_next = 1'b1;
	/* hs sync is 40 pixels long */
	end else begin
		reg_out_hs_next = 0;
	end

	/* if end of horizontal line */
	if(reg_out_hs_pixel_counter >= OUT_HORIZONTAL_PIXELS) begin
		
		/* if not in interlaced mode, then each line should be rendered twice, so simply reverse the read counter with one line for every second line displayed ! */
		if(reg_interlaced_active == 0) begin
			/* check if the line has been displayed twice and also make sure that we have started rendered (i.e. we are inside render area) since reversing read counter for every line is not what we want */
			if(reg_out_line_render_twice_flag == 0 && reg_sram_read_pixel_counter > 0) begin
				/* set the read counter to the start of the already displayed line */
				reg_sram_read_pixel_counter_next = reg_sram_read_pixel_counter - SRAM_WIDTH;
			end
			reg_out_line_render_twice_flag_next = !reg_out_line_render_twice_flag;
		end
		
		/* if end of vertical line */
		if(reg_out_line_counter >= OUT_VERTICAL_LINES) begin
			reg_out_line_counter_next = 0;
			reg_sram_read_pixel_counter_next = 0;
			reg_out_line_render_twice_flag_next = 0;
		end else begin
			reg_out_line_counter_next = reg_out_line_counter + 1'b1;
		end
		
		reg_out_hs_pixel_counter_next = 0;
		
	end

	/* after OUT_VBP lines, set vs out high for a duration of OUT_VSW (according to 720p timing spec) */
	if(reg_out_line_counter >= OUT_VBP && reg_out_line_counter < (OUT_VBP + OUT_VSW)) begin
		reg_out_vs_next = 1'b1;
	end else begin
		reg_out_vs_next = 0;
	end
end

/******************************************************************/
/************************* BUTTONS ********************************/
/******************************************************************/

/* button 1 */
always @ (negedge gpio_0)
begin
	reg_in_horizontal_blanking <= reg_in_horizontal_blanking_next;
	reg_in_vertical_blanking <= reg_in_vertical_blanking_next;
	reg_output_active <= reg_output_active_next;
end

always @*
begin
	/* default */
	reg_in_horizontal_blanking_next = reg_in_horizontal_blanking;
	reg_in_vertical_blanking_next = reg_in_vertical_blanking;
	reg_output_active_next = reg_output_active;
	
	if(gpio_1 == 0 && gpio_2 == 0 && gpio_3 == 0) begin
		reg_in_horizontal_blanking_next = IN_HORIZONTAL_BLANKING;
		reg_in_vertical_blanking_next = IN_VERTICAL_BLANKING;
		reg_output_active_next = 0;
	end else if(gpio_1 == 1 && gpio_2 == 0 && gpio_3 == 0) begin
		if(reg_in_horizontal_blanking != IN_HORIZONTAL_BLANKING_OFFSET) begin
			reg_in_horizontal_blanking_next = reg_in_horizontal_blanking - 4'd10;
		end
	end else if(gpio_1 == 1 && gpio_2 == 1 && gpio_3 == 0) begin
		if(reg_in_horizontal_blanking != (IN_HORIZONTAL_BLANKING + IN_HORIZONTAL_BLANKING_OFFSET)) begin
			reg_in_horizontal_blanking_next = reg_in_horizontal_blanking + 4'd10;
		end
	end else if(gpio_1 == 0 && gpio_2 == 1 && gpio_3 == 0) begin
		if(reg_in_vertical_blanking != IN_VERTICAL_BLANKING_OFFSET) begin
			reg_in_vertical_blanking_next = reg_in_vertical_blanking - 3'd5;
		end
	end else if(gpio_1 == 0 && gpio_2 == 1 && gpio_3 == 1) begin
		if(reg_in_vertical_blanking != (IN_VERTICAL_BLANKING + IN_VERTICAL_BLANKING_OFFSET)) begin
			reg_in_vertical_blanking_next = reg_in_vertical_blanking + 3'd5;
		end
	end else if(gpio_1 == 1 && gpio_2 == 1 && gpio_3 == 1) begin
		reg_output_active_next = 1;
	end
end

/******************************************************************/
/************************* ASSIGNMENTS ****************************/
/******************************************************************/

/* OUT */
assign out_hs = reg_out_hs;
assign out_pclk = reg_out_pclk;
assign out_vs = reg_out_vs;
assign out_de = 1'b1;
assign out_data = reg_out_data;

/* SRAM */
assign sram_ce_n = 1'b0; 
assign sram_ub_n = 1'b0;
assign sram_lb_n = 1'b0;
assign sram_oe_n = reg_sram_oe_n;
assign sram_we_n = reg_sram_in_wr_n;
assign sram_busy_n = reg_sram_out_busy_n;
assign sram_out_addr = reg_sram_in_addr;
assign sram_inout_data =  (!sram_we_n) ? reg_sram_in_data : 16'bz;
assign out_iled = reg_out_iled;

endmodule