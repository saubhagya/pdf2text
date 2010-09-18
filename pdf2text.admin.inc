<?php
// $Id: pdf2text.admin.inc,v 1.1 2010/06/04 08:00:14 saubhagya Exp $

/**
 * Menu callback for the upload settings form.
 */
function pdf2text_admin_settings() {
  // Get an array of node types with internal names as keys and
  // "friendly names" as values. E.g.,
  // array('page' => 'Page', 'story' => 'Story')
  $options = node_get_types('names');
  $form['pdf2text_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Users may select the content types'),
    '#options' => $options,
    '#default_value' => variable_get('pdf2text_node_types', array('page')),
    '#description' => t('A attachment field will be available on these content types to make user-specific notes.'),
  );
  return system_settings_form($form);
}
