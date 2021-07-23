#!/usr/bin/env ruby

puts "char hankaku[4096] = {"

File.open("hankaku.txt", mode = "rt"){|f|
  f.each_line(rs="",chomp: true){|para|
    if para.start_with?("char") then 
        print "\t"
        para.each_line(){|line|
            if !line.start_with?("char") then
                print "0x"
                print line.gsub('.','0').gsub('*','1').chomp.to_i(2).to_s(16)
                print ','
            end
        }
        puts ""
    else
        #print "=====================" + para
    end

  }
}

puts "};"