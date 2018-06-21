TYPE_CONTENT = 0x0;
TYPE_FLOAT = 0x1;
TYPE_INT = 0x2;
TYPE_STRING=0x4;
TYPE_HEX=0x8;
TYPE_BLOCK=0x10;

mkv={
	{id=0x1A45DFA3,name="EBML",type=TYPE_CONTENT},
	{id=0x18538067,name="Segment",type=TYPE_CONTENT},
}

function parse(id)
	for key,value in ipairs(mkv)
	do
		if (value.id == id)
		then
			return value;
		end
	end
	return nil;
end