#!/bin/bash
###############################################################################
# If parameter exist, suppose it is sram file to be decoded
#------------------------------------------------------------------------------
ret_u32=0
ret_u64=0
ret_str=foo.bin

read_u32() {
    ret_hex=0

    temp_buffer=`hd -s $1 -n 5 $sram_file | (read b1 b2 b3 b4 b5 b6;echo $b5$b4$b3$b2)`
    ret_hex=${temp_buffer}

    ((ret_u32=16#$ret_hex))
}

read_u64() {
    ret_hex=0

    temp_buffer=`hd -s $1 -n 9 $sram_file | (read b1 b2 b3 b4 b5 b6 b7 b8 b9 b10;echo $b9$b8$b7$b6$b5$b4$b3$b2)`
    ret_hex=${temp_buffer}

    ((ret_u64=16#$ret_hex))
}

read_name() {
    temp_buffer=`hd -e '20/1 "%c"' -s $1 -n 20 $sram_file`
    char_buffer=`expr substr "$temp_buffer" 80 20`
    ret_str=${char_buffer%%0000*}
}

read_buffer_to_file() {
    dd if=$sram_file of=$3 bs=1 count=$2 skip=$1
}

#------------------------------------------------------------------------------
##define MAX_RAMDUMP_TABLE_SIZE  6
#typedef struct
#{
#  uint64 base_address;
#  uint64 actual_phys_address;
#  uint64 size;
#  char description[20];
#  char file_name[20];
#}ramdump_entry;

#typedef struct
#{
#  uint32 version;
#  uint32 header_size;
#  ramdump_entry ramdump_table[MAX_RAMDUMP_TABLE_SIZE];
#}ramdump_header_t;

#ramdump_header_t rddm_dump_header =
#{
#  0x1, /*version*/
#  sizeof(ramdump_header_t),
#  {
#    Q6_SRAM_FULL_RAMDUMP_HEADER,
#    ETB_SOC_RAMDUMP_HEADER,
#    ETB_WCSS_RAMDUMP_HEADER,
#    M3_PHYA_RAMDUMP_HEADER,
#    M3_PHYB_RAMDUMP_HEADER,
#  }
#};

##define Q6_SRAM_FULL_RAMDUMP_HEADER     {SRAM_BASE_ADDRESS, SRAM_BASE_ADDRESS, SRAM_SIZE, "Q6-SRAM", "Q6-SRAM.bin"}
##define ETB_SOC_RAMDUMP_HEADER          {ETB_TEMP_BUFFER_ADDRESS, NULL, ETB_SOC_SIZE, "ETB_SOC_16K", "ETB_SOC.bin"}
##define ETB_WCSS_RAMDUMP_HEADER         {ETB_TEMP_BUFFER_ADDRESS, NULL, ETB_WCSS_SIZE, "ETB_WCSS_8K", "ETB_WCSS.bin"}
##define M3_PHYA_RAMDUMP_HEADER          {M3_PDMEM_TEMP_BUFFER_ADDR, M3_PHYA_PDMEM_BASE_ADDR, M3_PHYA_PDMEM_SIZE, "PHYA-M3", "PHYA-M3.3.bin"}
##define M3_PHYB_RAMDUMP_HEADER          {M3_PDMEM_TEMP_BUFFER_ADDR, M3_PHYB_PDMEM_BASE_ADDR, M3_PHYB_PDMEM_SIZE, "PHYB-M3", "PHYB-M3.3.bin"}
#------------------------------------------------------------------------------
sram_parser() {
    echo "Parsing $1 to get separate dump files..."

    header_offset=0
    file_offset=0

    # get version
    read_u32 $header_offset
    echo dump version $ret_u32
    let header_offset+=4

    # get header length
    read_u32 $header_offset
    file_offset=$ret_u32
    echo dump header length $file_offset
    let header_offset+=4

    for i in {1..6}; do
        tmp=0

        # get file length and file name
        let tmp=$header_offset+16
        read_u64 $tmp

        if [ $ret_u64 -ne 0 ]; then
            let tmp=$header_offset+44
            read_name $tmp
            echo file name $ret_str with length $ret_u64

            # write file
            read_buffer_to_file $file_offset $ret_u64 $ret_str
            let file_offset+=$ret_u64
        fi

        let header_offset+=64
    done

}

sram_file="fwsram.bin"

if [ $# -ne 0 ]; then
    if [ -n "$1" ]; then
        sram_file=$1
    fi
    if [ ! -e "$1" ]; then
        echo "$1 not exists!!!!!!"
        exit
    fi

    # Parse file
    sram_parser $1
    exit
fi
#------------------------------------------------------------------------------


###############################################################################
# Prepare VmCore file for crash
#------------------------------------------------------------------------------
vmcore_file="VmCore"
echo -n "VmCore path and filename? [${vmcore_file}]:"
read vmcore_file_in
if [ -n "${vmcore_file_in}" ]; then
    vmcore_file=${vmcore_file_in}
fi
if [ ! -e "${vmcore_file}" ]; then
    echo "${vmcore_file} not exists!!!!!!"
    exit
fi
echo ${vmcore_file} is existed.
#------------------------------------------------------------------------------


###############################################################################
# Prepare vmlinux file for crash
#------------------------------------------------------------------------------
vmlinux_file="vmlinux"
echo -n "vmlinux path and filename? [${vmlinux_file}]:"
read vmlinux_file_in
if [ -n "${vmlinux_file_in}" ]; then
    vmlinux_file=${vmlinux_file_in}
fi
if [ ! -e "${vmlinux_file}" ]; then
    echo "${vmlinux_file} not exists!!!!!!"
    exit
fi
echo ${vmlinux_file} is existed.
#------------------------------------------------------------------------------
rm -f *.bin *.log *.cmd

echo "Extracting the kernel log from the crash dump..."
echo "log > kern.log" > extract_kern_log.cmd
echo "quit" >> extract_kern_log.cmd
crash ${vmlinux_file} ${vmcore_file} -i extract_kern_log.cmd -s

echo "Extracting wlan firmware log from the crash dump..."

grep "\[firmware_dump\] to write" kern.log > firmware_dump.log;
grep "\[fw_paging_dump\] to write" kern.log > fw_paging_dump.log;
grep "\[fw_remote_mem_dump\] to write" kern.log > fw_remote_mem_dump.log

echo "Generate wlan firmware dumps..."
firmware_dump_file="fwsram.bin"
fw_paging_dump_file="paging.bin"
fw_remote_mem_dump_file="remote.bin"
tmp_file="tmp.bin"
command_file="rd.cmd"

echo "Generate wlan firmware remote dump..."
seg_addr=`grep "mem:" fw_remote_mem_dump.log|awk -F ": |," '{print $3}'`
seg_size=`grep "mem:" fw_remote_mem_dump.log|awk -F ": |," '{print $5}'`
echo "rd -x $seg_addr $seg_size -r ${fw_remote_mem_dump_file}" > ${command_file}
echo "quit" >> ${command_file}
crash ${vmlinux_file} ${vmcore_file} -i ${command_file} -s

echo "Generate wlan firmware sram dump..."
cat firmware_dump.log | while read line
do
seg_addr=`echo $line |awk -F ": |," '{print $3}'`
seg_size=`echo $line |awk -F ": |," '{print $5}'`
echo "rd -x $seg_addr $seg_size -r ${tmp_file}" > firmware_dump.cmd
echo "quit" >> firmware_dump.cmd
crash ${vmlinux_file} ${vmcore_file} -i firmware_dump.cmd -s
cat ${tmp_file} >> ${firmware_dump_file}
done

echo "Generate wlan firmware paging dump..."
cat fw_paging_dump.log | while read line
do
seg_addr=`echo $line |awk -F ": |," '{print $3}'`
seg_size=`echo $line |awk -F ": |," '{print $5}'`
echo "rd -x $seg_addr $seg_size -r ${tmp_file}" > fw_paging_dump.cmd
echo "quit" >> fw_paging_dump.cmd
crash ${vmlinux_file} ${vmcore_file} -i fw_paging_dump.cmd -s
cat ${tmp_file} >> ${fw_paging_dump_file}
done
echo "Generate wlan firmware dumps done"
echo "clear tmp files"
sram_parser ${firmware_dump_file}
rm -f tmp.bin *.log *.cmd
