<?php

require_once "helper.pdf2text.inc";

class PdfArray {
  private $array;
  private $elements;

  public function __construct($data) {
    if (isArrayType($data)) {
      $this->array = trim(substr($data, 1, strlen($data) - 2));
      $this->setElements();
    }
  }

  private function setElements() {
    if (preg_match_all("#([0-9]+ 0 R)#smU", $this->array, $matches)) {
      $this->elements = $matches[1];
    } elseif (preg_match_all("#/([a-zA-Z0-9]+)#sm", $this->array, $matches)) {
      $this->elements = $matches[1];
    } else {
      $this->elements = explode(" ", $this->array);
    }
  }

  public function getElements() {
    return $this->elements;
  }
}
