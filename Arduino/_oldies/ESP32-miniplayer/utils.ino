

String pullSplit(String& input, String separator) {
  String output;
  int pos = input.indexOf(separator);
  if (pos == -1) {
    output = String(input);
    input = "";
  }
  else {
    output = input.substring(0, pos-1);
    input = input.substring(pos);
  }

  return output;
}

String pad3(int input) {
  char bank[4];
  bank[3] = 0;
  bank[0] = '0' + input / 100;
  bank[1] = '0' + (input / 10) % 10;
  bank[2] = '0' + input % 10;
  return String(bank);
}

