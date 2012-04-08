<?php

require_once "helper.pdf2text.inc";

class Dictionary {
  private $dictionary;
  private $properties;

  public function __construct($data) {
    if (isDictionaryType($data)) {
      $this->dictionary = trim(substr($data, 2, strlen($data) - 4));
      $this->setProperties();
    }
  }

  private function getDictionaryTokens(&$tokens) {
    $openBrace = 0;
    $token = "";

    for ($i = 0; $i < strlen($this->dictionary); $i++) {
      $ch = $this->dictionary[$i];
      if ($ch == "/" && $openBrace == 0) {
        $state = 1;
        if (trim($token) != "") {
          $tokens[] = trim($token);
        }
        $token = "";
      } elseif ($ch == "[" || $ch == "<") {
        if ($openBrace == 0 && $state == 1) {
          $state = 2;
          if (trim($token) != "") {
            $tokens[] = trim($token);
          }
          $token = "";
        }
        $openBrace += 1;
        $token .= $ch;
      } elseif ($ch == "]" || $ch == ">") {
        $openBrace -= 1;
        $token .= $ch;
      }  elseif ($state == 1) {
        if ($ch == " " || $ch == "\n") {
          $state = 2;
          if (trim($token) != "") {
            $tokens[] = trim($token);
          }
          $token = "";
        } else {
          $token .= $ch;
        }
      } else {
        if ($ch == "\n") {
          $ch = " ";
        }
        $token .= $ch;
      }
    }

    if (trim($token) != "") {
      $tokens[] = trim($token);
    }
  }

  private function setProperties() {
    $tokens = array();

    $this->getDictionaryTokens($tokens);
    for ($i = 0; $i < count($tokens); $i += 2) {
      $this->properties[$tokens[$i]] = $tokens[$i + 1];
    }
  }

  public function getProperty($property) {
    if (isset($this->properties[$property])) {
      return $this->properties[$property];
    }
    return "";
  }
}
