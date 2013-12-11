<?php

  /**
   * @file
   * Block functionality for the module
   * @author griffinj@lafayette.edu
   *
   */

/**
 * @author griffinj@lafayette
 * Returns the top Solr facets for any given field
 * (Implemented in a manner similar to the ContentDM 4 advanced search feature)
 *
 * @see islandora_solr_facet_pages_callback
 *
 */

function _dss_eastasia_islandora_solr_facet_pages_block_callback() {

  // Retrieve the current path
  $path = current_path();

  // Explicitly set the prefix for all facets
  $prefix = t('ALL');

  // If the user agent is currently browsing a facet, render the filtered results...

  //if(preg_match('/browse\/(.+)/', $path, $m)) {
  //if(preg_match('/islandora\/search\/.*\?(.+)/', $path, $m)) {
  //if(preg_match('/browse\/(.+)/', $path, $m) || preg_match('/islandora\/search\/.*\?(.+)/', $path, $m)) {
  if(preg_match('/browse\/(.+)/', $path, $m) || preg_match('/islandora\/search\/(.+)/', $path, $m)) {

    $path = $m[1];
    $query_params = drupal_get_query_parameters();
    $filters = array();
    $filtered_facets = array();


    if($path == '*:*' && array_key_exists('f', $query_params)) {

      foreach($query_params['f'] as $filtered_facet) {

	$filters = $query_params['f'];

	if(preg_match('/"(.+)"/', $filtered_facet, $matches)) {

	  $filtered_facets[] = $matches[1]; 
	}
      }
    }

    // Get available fields from variable.
    $fields = variable_get('islandora_solr_facet_pages_fields_data', array());

    $solr_fields = array();

    // Select the field being faceted
    foreach($fields as $key => $value) {
      
      if($path != $value['path']) {
	
	$solr_fields[] = $value['solr_field'];
      }
    }

    $items = array();

    //foreach($solr_fields as $solr_field) {
    foreach($fields as $key => $value) {

      $solr_field = $value['solr_field'];

      // Add the field name itself
      // Terrible, refactor
      $items[] = "<div class='islandora-solr-facet'>{$value['label']}</div>";

      /*
      $items[] = render(array('div' => '',
			      'class' => 'islandora-solr-facet',
			      'content' => $value['label']));
      */
      
      // Use Solr faceting to get list of names.
      $parsed_url = parse_url(variable_get('islandora_solr_url', 'http://localhost:8080/solr'));

      $solr = new Apache_Solr_Service($parsed_url['host'], $parsed_url['port'], $parsed_url['path']);

      $query = "$solr_field:[* TO *]";

      if(!empty($filters)) {

	$query = implode(' AND ', $filters);

	// Collect results.
	$result_fields = _dss_eastasia_islandora_solr_facet_pages_results($solr, $solr_field, $prefix, $query);
      } else {
	
	// Collect results.
	$result_fields = _dss_eastasia_islandora_solr_facet_pages_results($solr, $solr_field, $prefix);
      }

      // Collect results with lowercase.
      $prefix_lower = strtolower($prefix);

      if(!empty($filters)) {

	$query = implode(' AND ', $filters);

	$result_fields_lower = _dss_eastasia_islandora_solr_facet_pages_results($solr, $solr_field, $prefix_lower, $query);
      } else {
	
	$result_fields_lower = _dss_eastasia_islandora_solr_facet_pages_results($solr, $solr_field, $prefix_lower);
      }
      // Merge uppercase with lowercase.
      $result_fields = array_merge($result_fields, $result_fields_lower);

      $child_items = array();
      if(!empty($result_fields)) {

	// Sort by the most populated facets
	arsort($result_fields);
	
	// Top 5 facets
	$result_fields = array_slice($result_fields, 0, 5);

	foreach($result_fields as $result => $count) {

	  if(!in_array($result, $filtered_facets)) {

	    $filter = array_merge($filters, array($solr_field . ':"' . addslashes($result) . '"'));

	    // For date values
	    if(preg_match('/Date\./', $value['label'])) {

	      $child_items[] = l(truncate_utf8(date('F, Y', strtotime($result)), 72, TRUE, TRUE), 'islandora/search/*:*', array('query' => array('f' => $filter))) . "&nbsp;<span class='bucket-size'>($count)</span>";
	    } else {

	      $child_items[] = l(truncate_utf8($result, 72, TRUE, TRUE), 'islandora/search/*:*', array('query' => array('f' => $filter))) . "&nbsp;<span class='bucket-size'>($count)</span>";
	    }
	  }
	}

	$child_items[] = l('Show more...', "browse/{$value['path']}");
      } else {

	$child_items[] = l('View all values...', "browse/{$value['path']}");
      }

      if(!empty($child_items)) {

	$items[] = theme('item_list', array('items' => $child_items,
					    'type' => 'ul',
					    'attributes' => array('class' => 'islandora-solr-facet-list',
								  )));
      }
    }

    if(!empty($items)) {

      $results = implode($items);
      return $results;
    } else {
      
      return '';
    }

  } else {

    // Get available fields from variable.
    $fields = variable_get('islandora_solr_facet_pages_fields_data', array());
  
    $items = array();
    foreach ($fields as $key => $value) {

      $items[] = l($value['label'], 'browse/' . $value['path'], array(
								      'attributes' => array(
											    'title' => $value['label'],
											    ),
								      ));
    }

    if(!empty($items)) {

      return theme('item_list', array(
				      'items' => $items,
				      'type' => 'ul',
				      'attributes' => array(
							    'class' => 'islandora-solr-facet-filter-list',
							    ),
				      ));
    } else {
      
      return FALSE;
    }
  }
}

/**
 * Implements hook_block_view().
 */
function dss_eastasia_block_view_alter(&$data, $block) {

  switch ($block->delta) {
  case 'islandora-solr-facet-pages':

    $path = current_path();
    //if(preg_match('/browse\//', $path) || preg_match('/islandora\/search/', $path)) {
    if(preg_match('/islandora\/search/', $path)) {

      #$path = current_path();
      $data['title'] = 'Refine your search';
      $data['content'] = _dss_eastasia_islandora_solr_facet_pages_block_callback();
      break;
    }
  }
}