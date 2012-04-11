<?php

class TextContainer {
  private $textContainer;
  private $fontObj;
  
  public function __construct($data, $fontObj) {
    $this->textContainer = trim($data);
    $this->fontObj = $fontObj;
  }
  
  public function getText() {
    $text = "";
    
    if (preg_match_all("#\((.*)\)#smU", $this->textContainer, $matches)) {
      $contents = $matches[1];
      foreach ($contents as $content) {
        $text .= $content;
      }
    }
    return $text;
  }
}