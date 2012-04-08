<?php

class Decoder {
  private $data;

  private function asciihexDecode() {
    $input = $this->data;
    $output = "";
    $isOdd = TRUE;
    $isComment = FALSE;

    for ($i = 0, $codeHigh = -1; $i < strlen($input) && $input[$i] != '>'; $i++) {
      $c = $input[$i];
      if ($isComment) {
        if ($c == '\r' || $c == '\n') {
          $isComment = FALSE;
        }
        continue;
      }

      switch ($c) {
        case '\0': case '\t': case '\r': case '\f': case '\n': case ' ':
          break;
        case '%':
          $isComment = TRUE;
          break;
        default:
          $code = hexdec($c);
          if ($code === 0 && $c != '0') {
            return "";
          }
          if ($isOdd) {
            $codeHigh = $code;
          } else {
            $output .= chr($codeHigh * 16 + $code);
          }
          $isOdd = !$isOdd;
          break;
      }
    }

    if ($input[$i] != '>') {
      return "";
    }
    if ($isOdd) {
      $output .= chr($codeHigh * 16);
    }
    return $output;
  }

  private function ascii85Decode() {
    $input = $this->data;
    $output = "";
    $isComment = FALSE;
    $ords = array();

    for ($i = 0, $state = 0; $i < strlen($input) && $input[$i] != '~'; $i++) {
      $c = $input[$i];

      if ($isComment) {
        if ($c == '\r' || $c == '\n') {
          $isComment = FALSE;
        }
        continue;
      }

      if ($c == '\0' || $c == '\t' || $c == '\r' || $c == '\f' || $c == '\n' || $c == ' ') {
        continue;
      }
      if ($c == '%') {
        $isComment = TRUE;
        continue;
      }
      if ($c == 'z' && $state === 0) {
        $output .= str_repeat(chr(0), 4);
        continue;
      }
      if ($c < '!' || $c > 'u') {
        return "";
      }

      $code = ord($input[$i]) & 0xff;
      $ords[$state++] = $code - ord('!');

      if ($state == 5) {
        $state = 0;
        for ($sum = 0, $j = 0; $j < 5; $j++) {
          $sum = $sum * 85 + $ords[$j];
        }
        for ($j = 3; $j >= 0; $j--) {
          $output .= chr($sum >> ($j * 8));
        }
      }
    }

    if ($state === 1) {
      return "";
    } elseif ($state > 1) {
      for ($i = 0, $sum = 0; $i < $state; $i++) {
        $sum += ($ords[$i] + ($i == $state - 1)) * pow(85, 4 - $i);
      }
      for ($i = 0; $i < $state - 1; $i++) {
        $ouput .= chr($sum >> ((3 - $i) * 8));
      }
    }
    return $output;
  }

  private function flateDecode() {
    $input = $this->data;
    return @gzuncompress($input);
  }

  public function __construct($data) {
    $this->data = $data;
  }

  public function decode($filter) {
    switch ($filter) {
      case "ASCIIHexDecode":
        $decodedData = $this->asciihexDecode();
        break;
      case "ASCII85Decode":
        $decodedData = $this->ascii85Decode();
        break;
      case "FlateDecode":
        $decodedData = $this->flateDecode();
        break;
      default:
        $decodedData = $this->data;
    }
    return $decodedData;
  }
}
