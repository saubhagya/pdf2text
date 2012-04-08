<?php

function isArrayType($value) {
  $length = strlen($value);
  
  if ($value[0] == "[" && $value[$length - 1] == "]") {
    return true;
  }
  return false;
}

function isDictionaryType($value) {
  $length = strlen($value);
  
  if ($length >= 4 && $value[0] == "<" && $value[1] == "<"
  && $value[$length - 1] == ">" && $value[$length - 2] == ">") {
    return true;
  }
  return false;
}
