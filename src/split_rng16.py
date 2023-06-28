filesToSplitTo = [
  "xor16_11e.cpp", "xor16_11f.cpp", "xor16_152.cpp", "xor16_174.cpp", "xor16_17b.cpp", "xor16_1b3.cpp", "xor16_1f6.cpp", "xor16_1f7.cpp", "xor16_251.cpp", "xor16_25d.cpp",
  "xor16_25f.cpp", "xor16_27d.cpp", "xor16_27f.cpp", "xor16_31c.cpp", "xor16_31f.cpp", "xor16_35b.cpp", "xor16_3b1.cpp", "xor16_3bb.cpp", "xor16_3d9.cpp", "xor16_437.cpp",
  "xor16_471.cpp", "xor16_4bb.cpp", "xor16_57e.cpp", "xor16_598.cpp", "xor16_5b6.cpp", "xor16_5bb.cpp", "xor16_67d.cpp", "xor16_6b5.cpp", "xor16_6f1.cpp", "xor16_71b.cpp",
  "xor16_734.cpp", "xor16_798.cpp", "xor16_79d.cpp", "xor16_7f1.cpp", "xor16_895.cpp", "xor16_897.cpp", "xor16_97d.cpp", "xor16_9d3.cpp", "xor16_b17.cpp", "xor16_b3d.cpp",
  "xor16_b53.cpp", "xor16_b71.cpp", "xor16_bb3.cpp", "xor16_bb4.cpp", "xor16_bb5.cpp", "xor16_c13.cpp", "xor16_d3b.cpp", "xor16_c3d.cpp", "xor16_d3c.cpp", "xor16_d52.cpp",
  "xor16_d72.cpp", "xor16_d76.cpp", "xor16_d79.cpp", "xor16_d97.cpp", "xor16_e11.cpp", "xor16_e75.cpp", "xor16_f11.cpp", "xor16_f13.cpp", "xor16_f52.cpp", "xor16_f72.cpp"]
f = open("rng16cmpl.c.blob", 'r')
filetext=f.read();
f.close()
filecontents=filetext.split("@")
for x in filesToSplitTo:
  fout = open(x, 'w')
  fout.write(filecontents.pop(0))
  fout.close()
