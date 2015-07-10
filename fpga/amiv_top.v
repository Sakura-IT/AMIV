

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
				 input wire gpio_3);

parameter IN_FRAME_PER_SECOND = 50; /* number of frames used by interlaced detection */
parameter IN_HORIZONTAL_BLANKING = 170;
parameter IN_VERTICAL_BLANKING = 30;

parameter IN_HORIZONTAL_BLANKING_OFFSET = 40;
parameter IN_VERTICAL_BLANKING_OFFSET = 15;

parameter OUT_HFP = 440; /* set to 110 for 720p @ 60Hz and to 440 for 720p @ 50Hz */;
parameter OUT_HBP = 220;
parameter OUT_HSW = 40;
parameter OUT_VFP = 5;
parameter OUT_VBP = 20;
parameter OUT_VSW = 5;
parameter OUT_WIDTH = 1280;
parameter OUT_HEIGHT = 720;
parameter OUT_HORIZONTAL_BLANKING = OUT_HFP + OUT_HBP + OUT_HSW;
parameter OUT_VERTICAL_BLANKING = OUT_VFP + OUT_VBP + OUT_VSW;
parameter OUT_HORIZONTAL_PIXELS = OUT_WIDTH + OUT_HORIZONTAL_BLANKING;
parameter OUT_VERTICAL_LINES = OUT_HEIGHT + OUT_VERTICAL_BLANKING;
parameter OUT_WIDTH_OFFSET = 0;
parameter OUT_HEIGHT_OFFSET = 78; /* 720p got more lines than input format, in order to center picture, this is offset counted from the top of the screen */

parameter SRAM_WIDTH = 640;
parameter SRAM_HEIGHT = 580;

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
reg [15:0] reg_out_hs_pixel_counter;
reg [15:0] reg_out_hs_pixel_counter_next;
reg [11:0] reg_out_line_counter;
reg [11:0] reg_out_line_counter_next;

/* IN */
reg [29:0] reg_in_data_saved;
reg [29:0] reg_in_data_saved_next;
reg [15:0] reg_in_hs_pixel_counter;
reg [15:0] reg_in_hs_pixel_counter_next;
reg [11:0] reg_in_line_counter;
reg [11:0] reg_in_line_counter_next;
reg [8:0] reg_in_horizontal_blanking;
reg [8:0] reg_in_horizontal_blanking_next;
reg [7:0] reg_in_vertical_blanking;
reg [7:0] reg_in_vertical_blanking_next;
reg reg_interlaced_active; /* this is set if the mode is interlaced */
reg reg_interlaced_active_next;
reg reg_interlaced_detected; /* this will be set when odd/even field has switched from high to low during number of frames set in reg_interlaced_active_counter */
reg reg_interlaced_detected_next;
reg [7:0] reg_interlaced_active_counter; /* used to detect interlaced mode */
reg [7:0] reg_interlaced_active_counter_next;

/* MISC */
reg [3:0] reg_state;
reg [3:0] state_next;
wire mclk;
reg reg_frame_counter;
reg reg_frame_counter_next;
reg reg_reset_screen_pos = 0;

localparam [2:0] state_0 = 3'b000,
				 state_1 = 3'b001,
				 state_2 = 3'b010,
				 state_3 = 3'b011;


/* use ref clk x 4 to obtain a main clk of 148,5Mhz */
AMIV_PLL pll( .CLKI(ref_clock),
              .CLKOP(mclk) );  
	
/* when clk in goes high, write counter and pixel counter are updated and corresponding data is saved */
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
		reg_in_hs_pixel_counter_next = reg_in_hs_pixel_counter + 1;
	end

	/* if counters say that we are inside the window that should be rendered */
	if(reg_in_hs_pixel_counter >= reg_in_horizontal_blanking &&
			reg_in_hs_pixel_counter < (SRAM_WIDTH + reg_in_horizontal_blanking) &&
			reg_in_line_counter >= reg_in_vertical_blanking &&
			reg_in_line_counter < (SRAM_HEIGHT/2 + reg_in_vertical_blanking)) begin
		if(reg_interlaced_active == 1) begin
			/* even or odd line ? */
			if( in_oddeven == 0) begin // even line
				reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
													(reg_in_line_counter - reg_in_vertical_blanking) * 2 * SRAM_WIDTH;
			end else begin
				reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
													(reg_in_line_counter - reg_in_vertical_blanking) * 2 * SRAM_WIDTH + SRAM_WIDTH;
			end
		end else begin
			/* write pixel sequentially, the reading procedure will output every row 2 times */
			reg_sram_write_pixel_counter_next = (reg_in_hs_pixel_counter - reg_in_horizontal_blanking) +
												(reg_in_line_counter - reg_in_vertical_blanking) * SRAM_WIDTH;
		end
	end

	/* store the data here, so that it can be written inside main statemachine */
	reg_in_data_saved_next = in_data;
end


always @ (posedge in_vs)
begin
	reg_frame_counter <= reg_frame_counter_next;

	/* take care of the interlaced mode, e.g. if odd/even field did not go low
	 * during x frames then it is not interlaced */
	if(reg_interlaced_active_counter > 0) begin
		reg_interlaced_active_counter <= reg_interlaced_active_counter - 1;

		if(in_oddeven == 0) begin
			reg_interlaced_detected <= 1;
		end

	end else begin
		reg_interlaced_active_counter <= IN_FRAME_PER_SECOND;
		if(reg_interlaced_detected == 1) begin
			reg_interlaced_active <= 1;
		end else begin
			reg_interlaced_active <= 0;
		end
		
		reg_interlaced_detected <= 0;
		
	end
end

always @*
begin
	reg_frame_counter_next = !reg_frame_counter;
end

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
		reg_in_line_counter_next = reg_in_line_counter + 1;
		
	end
end

always @ (negedge mclk)
begin
	
end

/* main state machine */
always @ (posedge mclk)
begin
	reg_state <= state_next;
	reg_out_data <= reg_out_data_next;
	reg_out_pclk <= reg_out_pclk_next;
	reg_out_hs_pixel_counter <= reg_out_hs_pixel_counter_next;
	reg_sram_read_pixel_counter <= reg_sram_read_pixel_counter_next;
	reg_sram_in_wr_n <= reg_sram_in_wr_n_next;
	reg_sram_in_addr <= reg_sram_in_addr_next;
	reg_sram_in_data <= reg_sram_in_data_next;
	reg_out_hs <= reg_out_hs_next;
	reg_out_line_counter <= reg_out_line_counter_next;
	reg_out_de <= reg_out_de_next;
	reg_out_vs <= reg_out_vs_next;
	reg_sram_oe_n <= reg_sram_oe_n_next;
	reg_out_line_render_twice_flag <= reg_out_line_render_twice_flag_next;
	
	/* make sure that new pixel is written to sram only when it appropriate */
	if(in_pclk == 1 && reg_sram_flag_write == 0) begin
		reg_sram_flag_write <= 1;
	end else begin
		reg_sram_flag_write <= reg_sram_flag_write_next;
	end
end

always @*
begin
	/* default */
	state_next = state_0;
	reg_out_data_next = reg_out_data;
	reg_out_pclk_next = reg_out_pclk;
	reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter;
	reg_sram_read_pixel_counter_next = reg_sram_read_pixel_counter;
	reg_sram_in_wr_n_next = 1'b1;
	reg_sram_in_addr_next = reg_sram_in_addr;
	reg_sram_in_data_next = reg_sram_in_data;
	reg_out_line_counter_next = reg_out_line_counter;
	reg_out_hs_next = reg_out_hs;
	reg_out_de_next = reg_out_de;
	reg_out_vs_next = reg_out_vs;
	reg_sram_oe_n_next = 1'b0;
	reg_sram_flag_write_next = reg_sram_flag_write;
	reg_out_line_render_twice_flag_next = reg_out_line_render_twice_flag;

	case(reg_state)
	
	state_0:
		begin
			state_next = state_1;
			reg_out_pclk_next = 1;
			
			/* time to increase pixel counter out, this should only happen mclk/2 */
			reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter + 1;		
		end
	state_1:
		begin
			state_next = state_2;
			
			/* time to clock out data read from sram */
			reg_out_pclk_next = 0;

			/* put the data that was read from sram on the out buffer */
			/* if counters say that we are inside the window that should be rendered */
			if(reg_out_hs_pixel_counter >= OUT_HORIZONTAL_BLANKING &&
			   reg_out_hs_pixel_counter < ((SRAM_WIDTH * 2) + OUT_HORIZONTAL_BLANKING) &&
			   reg_out_line_counter >= (OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET) &&
			   reg_out_line_counter < (SRAM_HEIGHT + OUT_VERTICAL_BLANKING + OUT_HEIGHT_OFFSET - (reg_in_vertical_blanking - IN_VERTICAL_BLANKING_OFFSET)*2)) begin
				reg_out_data_next[23:19] = sram_inout_data[15:11];
				reg_out_data_next[18:16] = 0;
				reg_out_data_next[15:10] = sram_inout_data[10:5];
				reg_out_data_next[9:8] = 0;
				reg_out_data_next[7:3] = sram_inout_data[4:0];
				reg_out_data_next[2:0] = 0;
			/* if not then just send black color */
			end else begin
				reg_out_data_next[23:19] = 0;
				reg_out_data_next[18:16] = 0;
				reg_out_data_next[15:10] = 0;
				reg_out_data_next[9:8] = 0;
				reg_out_data_next[7:3] = 0;
				reg_out_data_next[2:0] = 0;
			end
			
			/* time to write data to sram ? */
			if(reg_sram_flag_write == 1) begin
				reg_sram_flag_write_next = 0;
				reg_sram_in_wr_n_next = 1'b0;
				
				/* set the write address */
				reg_sram_in_addr_next = reg_sram_write_pixel_counter;
			end else begin
				reg_sram_in_wr_n_next = 1'b1;
			end
		end
	state_2:
		begin
			state_next = state_3;
			reg_out_pclk_next = 1;
			
			/* time to increase pixel counter out, this should only happen mclk/2 */
			reg_out_hs_pixel_counter_next = reg_out_hs_pixel_counter + 1;
			
			/* dummy section start (in order to configure the pins they must be "used") */
			if(reg_in_data_saved[24] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[23] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[22] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[21] == 1)
				reg_sram_out_busy_n = 0;
			else if(reg_in_data_saved[20] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[13] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[12] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[11] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[10] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[4] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[3] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[2] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[1] == 1)
				reg_sram_out_busy_n = 1;
			else if(reg_in_data_saved[0] == 1)
				reg_sram_out_busy_n = 0;
			else if(in_oddeven == 1)
				reg_sram_out_busy_n = 0;
			
			if(gpio_2 == 0) begin
				reg_sram_out_busy_n = 1;
			end
			/* dummy section end */
				
			/* put the data that should be written on sram bus (round up if necessary) */
			reg_sram_in_data_next[15:11] = reg_in_data_saved[29:25];
			
			/* round up ? */
			/*if(reg_sram_in_data_next[15:11] != 5'b11111) begin
				if(reg_in_data_saved[24:20] >= 5'b10000) begin
					reg_sram_in_data_next[15:11] = reg_sram_in_data_next[15:11] + 1;
				end
			end*/
			
			
			reg_sram_in_data_next[10:5] = reg_in_data_saved[19:14];
			
			/*if(reg_sram_in_data_next[10:5] != 6'b111111) begin
				if(reg_in_data_saved[13:10] >= 4'b1000) begin
					reg_sram_in_data_next[10:5] = reg_sram_in_data_next[10:5] + 1;
				end
			end*/
			
			reg_sram_in_data_next[4:0] = reg_in_data_saved[9:5];

			/*if(reg_sram_in_data_next[4:0] != 5'b11111) begin
				if(reg_in_data_saved[4:0] >= 5'b10000) begin
					reg_sram_in_data_next[4:0] = reg_sram_in_data_next[4:0] + 1;
				end
			end*/


			/* the default value will set reg_sram_in_wr_n_next back to 1 and thus, data will be written */
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
				reg_sram_read_pixel_counter_next = reg_sram_read_pixel_counter + 1;
				
				/* set the read address */
				reg_sram_in_addr_next = reg_sram_read_pixel_counter;
			end
		end
	default:
		begin
			state_next = state_0;
		end
	endcase

	/* after OUT_HFP pixels, set hs out high for a duration of OUT_HSW (according to 720p timing spec) */
	if(reg_out_hs_pixel_counter >= OUT_HFP && reg_out_hs_pixel_counter < (OUT_HFP + OUT_HSW)) begin
		reg_out_hs_next = 1;
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
			reg_out_line_counter_next = reg_out_line_counter + 1;
		end
		
		reg_out_hs_pixel_counter_next = 0;
		
	end

	/* after OUT_VBP lines, set vs out high for a duration of OUT_VSW (according to 720p timing spec) */
	if(reg_out_line_counter >= OUT_VBP && reg_out_line_counter < (OUT_VBP + OUT_VSW)) begin
		reg_out_vs_next = 1;
	end else begin
		reg_out_vs_next = 0;
	end
	

end

/* button 1 */
always @ (negedge gpio_0)
begin
	reg_in_horizontal_blanking <= reg_in_horizontal_blanking_next;
end

/* button 2 */
always @ (negedge gpio_1)
begin
	reg_in_vertical_blanking <= reg_in_vertical_blanking_next;
end

always @*
begin
	/* default */
	reg_in_horizontal_blanking_next = reg_in_horizontal_blanking;
	reg_in_vertical_blanking_next = reg_in_vertical_blanking;
	
	if(gpio_3 == 0) begin
		reg_in_horizontal_blanking_next = IN_HORIZONTAL_BLANKING;
	end else if(reg_in_horizontal_blanking == (IN_HORIZONTAL_BLANKING - IN_HORIZONTAL_BLANKING_OFFSET)) begin
		reg_in_horizontal_blanking_next = (IN_HORIZONTAL_BLANKING + IN_HORIZONTAL_BLANKING_OFFSET*2);
	end else begin
		reg_in_horizontal_blanking_next = reg_in_horizontal_blanking - 10;
	end
	
	if(gpio_3 == 0) begin
		reg_in_vertical_blanking_next = IN_VERTICAL_BLANKING;
	end else if(reg_in_vertical_blanking == (IN_VERTICAL_BLANKING - IN_VERTICAL_BLANKING_OFFSET)) begin
		reg_in_vertical_blanking_next = (IN_VERTICAL_BLANKING + IN_VERTICAL_BLANKING_OFFSET*2);
	end else begin
		reg_in_vertical_blanking_next = reg_in_vertical_blanking - 5;
	end
end

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

endmodule