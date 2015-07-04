
module AMIV_SRAM( input wire in_clk,
                  input wire in_reset_n,
                  input wire in_start_n,
                  input wire in_rw,
				  input wire in_fast_write,
                  input wire [18:0] in_addr,
                  input wire [15:0] in_data,
                  output wire out_we_n,
				  output wire out_oe_n,
                  output wire out_ce_n,
				  output wire out_ub_n,
				  output wire out_lb_n,
                  output reg out_busy_n,
				  output wire [18:0] out_addr,
                  output wire [15:0] out_data,
                  inout wire [15:0] io_data );

//states
localparam [1:0]  state_idle  = 3'b00,
                  state_read  = 3'b01,
                  state_write = 3'b10;

reg [2:0]   state_reg, state_next;
reg [15:0]  in_data_reg,   in_data_next;
reg [15:0]  out_data_reg, out_data_next;
reg [18:0]  addr_reg, addr_next;
reg we_next, oe_next, tri_next;
reg we_reg, oe_reg, tri_reg;

always@(negedge in_clk)
	begin
		state_reg <= state_next;
		addr_reg  <= addr_next;
		in_data_reg <= in_data_next;
		out_data_reg <= out_data_next;
		we_reg <= we_next;
		oe_reg <= oe_next;
		tri_reg <= tri_next;
	end
		
//next state values and outputs
always@*
begin
	addr_next = addr_reg;
	in_data_next = in_data_reg;
	out_data_next = out_data_reg;
	out_busy_n = 1'b0;
	oe_next = 1'b1;
	we_next = 1'b1;
	tri_next = 1'b1;

case(state_reg)		
	state_idle:
	begin
		out_busy_n = 1'b1;
		oe_next = 1'b1;
		if(in_start_n)  						
			state_next = state_reg;
		else
			begin
			  addr_next = in_addr;
			  if(in_rw == 1)
				begin
					state_next = state_read;  //BEGIN READ PROCESS
					oe_next = 1'b0;   
				end	
			  else
			   begin  								
					state_next = state_write;	// BEGIN WRITE PROCESS
					in_data_next = in_data;
					we_next = 1'b0;
					oe_next = 1'b1;
					tri_next = 1'b0;
				end
			end
	end
	state_read:
		begin
			state_next = state_idle;
			out_data_next = io_data;
			oe_next = 1'b1;
			if(in_fast_write)
			   begin  								
					state_next = state_write;	// BEGIN WRITE PROCESS
					in_data_next = in_data;
					we_next = 1'b0;
					tri_next = 1'b0;
				end
		end
	state_write:
		begin
			state_next = state_idle;
			tri_next = 1'b0;
			if(in_fast_write)
				begin
					state_next = state_read;  //BEGIN READ PROCESS
					oe_next = 1'b0;   
				end
		end
	default: 
		state_next = state_idle;
endcase
end

//outputs
assign out_ce_n = 1'b0; 
assign out_ub_n = 1'b0;
assign out_lb_n = 1'b0;
assign out_oe_n = oe_reg;
assign out_we_n = we_reg;
assign out_addr = addr_reg;
assign out_data = out_data_reg;

assign io_data =  (!tri_reg) ? in_data_reg : 16'bz;
endmodule
