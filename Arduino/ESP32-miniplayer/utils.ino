

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

