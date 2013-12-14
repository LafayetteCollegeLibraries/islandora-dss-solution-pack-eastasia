<?php

module_load_include('inc', 'dss_eastasia', 'includes/eastAsiaMods');

  /**
   * @file
   * Theming hooks
   * @author griffinj@lafayette.edu
   *
   */

function _dss_eastasia_preprocess_dss_eastasia_page(array &$variables, $page) {

  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $object = $variables['object'];

  // View Links.
  $display = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $grid_active = ($display == 'grid') ? 'active' : '';
  $list_active = ($display == 'active') ? 'active' : '';
  $query_params = drupal_get_query_parameters($_GET);
  $variables['view_links'] = array(
    array(
      'title' => 'Grid view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-grid $grid_active",
      ),
      'query' => $query_params + array('display' => 'grid'),
    ),
    array(
      'title' => 'List view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-list $list_active",
      ),
      'query' => $query_params + array('display' => 'list'),
    ),
  );

  // Get token to allow access to XACML protected datastreams.
  // Always use token authentication in case there is a global policy.
  module_load_include('inc', 'islandora', 'includes/authtokens');
  $token = islandora_get_object_token($page['pid'], 'JP2', 2);

  $jp2_url = url("islandora/object/{$page['pid']}/datastream/JP2/view",
		 array(
		       'absolute' => TRUE,
		       'query' => array('token' => $token),
		       ));
  $params = array(
		  'jp2_url' => $jp2_url,
		  );

  module_load_include('inc', 'islandora', 'includes/solution_packs');

  $viewer = islandora_get_viewer($params, 'islandora_book_page_viewers');

  dpm($viewer);

  if($viewer) {
    
    $variables['viewer'] = $viewer;
  }

  // Metadata
  /*
  try {

    $mods_str = $object['MODS']->content;
    $mods_object = new Mods($mods_str);
  } catch (Exception $e) {
    
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }

  $variables['mods_object'] = isset($mods_object) ? $mods_object->mods : (object) array();
  */
  }

/*
 * Serves as the "front" tab
 */
function dss_eastasia_preprocess_dss_eastasia_postcard(array &$variables) {

  module_load_include('inc', 'islandora', 'includes/solution_packs');
  //module_load_include('inc', 'dss_eastasia', 'includes/mods');
  $object = $variables['object'];

  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $pages = islandora_paged_content_get_pages($object);
  if(!empty($pages)) {

    $page = array_shift($pages);

    // Get token to allow access to XACML protected datastreams.
    // Always use token authentication in case there is a global policy.
    module_load_include('inc', 'islandora', 'includes/authtokens');
    $token = islandora_get_object_token($page['pid'], 'JP2', 2);

    $jp2_url = url("islandora/object/{$page['pid']}/datastream/JP2/view",
		   array(
			 'absolute' => TRUE,
			 'query' => array('token' => $token),
			 ));
    $params = array(
		    'jp2_url' => $jp2_url,
		    );

    $viewer = islandora_get_viewer($params, 'islandora_book_page_viewers');
    if ($viewer) {

      $variables['viewer'] = $viewer;
    }
  } else {

    unset($variables['viewer']);
  }

  /*
  $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $variables['book_object_id'] = $results[0]['object']['value'];
  */
  // Get token to allow access to XACML protected datastreams.
  // Always use token authentication in case there is a global policy.
  /*
  module_load_include('inc', 'islandora', 'includes/authtokens');
  $token = islandora_get_object_token($object->id, 'JP2', 2);
  $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view",
             array(
               'absolute' => TRUE,
               'query' => array('token' => $token),
             ));

  $params = array(
    'jp2_url' => $jp2_url,
  );
  */
  
  // Refactor
  // Retrieve the MODS Metadata
  try {

    $mods_str = $object['MODS']->content;

    $mods_str = preg_replace('/<\?xml version="1.0"\?>/', '', $mods_str);
    $mods_str = '<modsCollection>' . $mods_str . '</modsCollection>';

    $mods_object = new EastAsiaMods($mods_str);
  } catch (Exception $e) {
    
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }

  $variables['mods_object'] = isset($mods_object) ? $mods_object->mods : array();
}

  /**
   * Implements hook_preprocess_theme().
   */
function dss_eastasia_preprocess_dss_eastasia_last_page(array &$variables) {

  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $object = $variables['object'];
  $pages = islandora_paged_content_get_pages($object);

  // Refactor
  // Retrieve the MODS Metadata
  //module_load_include('inc', 'dss_eastasia', 'includes/eastAsiaMods');
  try {

    $mods_str = $object['MODS']->content;

    $mods_str = preg_replace('/<\?xml version="1.0"\?>/', '', $mods_str);
    $mods_str = '<modsCollection>' . $mods_str . '</modsCollection>';

    $mods_object = new EastAsiaMods($mods_str);
  } catch (Exception $e) {
    
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }

  $variables['mods_object'] = isset($mods_object) ? $mods_object->mods : array();

  if(!empty($pages)) {

    $last_page = array_pop($pages);

    _dss_eastasia_preprocess_dss_eastasia_page($variables, $last_page);
  }
}

function dss_eastasia_preprocess_dss_eastasia_front_page(array &$variables) {

  dpm('trace');

  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  //module_load_include('inc', 'dss_eastasia', 'includes/mods');

  $object = $variables['object'];
  $pages = islandora_paged_content_get_pages($object);

  // Refactor
  // Retrieve the MODS Metadata
  try {

    $mods_str = $object['MODS']->content;

    $mods_str = preg_replace('/<\?xml version="1.0"\?>/', '', $mods_str);
    $mods_str = '<modsCollection>' . $mods_str . '</modsCollection>';

    $mods_object = new EastAsiaMods($mods_str);
  } catch (Exception $e) {
    
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }

  $variables['mods_object'] = isset($mods_object) ? $mods_object->mods : array();

  if(!empty($pages)) {

    $last_page = array_shift($pages);
    
    _dss_eastasia_preprocess_dss_eastasia_page($variables, $last_page);
  }

  /*
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $object = $variables['object'];
  $pages = islandora_paged_content_get_pages($object);

  if(!empty($pages)) {

    $first_page = array_shift($pages);

    _dss_eastasia_preprocess_dss_eastasia_page($variables, $first_page);
  }
  */

}

function dss_eastasia_preprocess_islandora_solr_facet_pages_letterer(&$vars) {

  dpm($vars);
  extract($vars);

  switch($vars['path']) {

  case 'eastasia-subject':

    /*
    foreach (range(1, 9) as $letter) {

      $value = "$solr_field:$letter*";
      $fq_map[$value] = $letter;
      $fq[] = $value;
    }
    */
    dpm($fq_map);
    break;
  }

  $output = '<ul class="islandora-solr-facet-pages-letterer">';

  $output .= '<li class="letter first' . ((t('ALL') == $prefix) ? ' active' : '') . '">';
  $output .= '<a href="' . url('browse/' . $path) . '" title="' . t('Browse all') . '">' . t('ALL') . '</a>';
  $output .= '</li>';

  // Loop over facet queries to render letters.
  foreach ($facet_queries as $query => $count) {
    $value = $fq_map[$query];

    $output .= '<li class="letter' . (($value == $prefix) ? ' active' : '') . '">';

    // Create link if facet bucket is not empty.
    if ($count > 0) {
      $url = 'browse/' . $path . '/' . $value;
      $output .= '<a href="' . url($url) . '" title="' . t('Browse starting with @letter', array('@letter' => $value)) . '">' . $value . '</a>';
    }
    else {
      $output .= $value;
    }
    $output .= '</li>';
  }
  $output .= '</ul>';

  return $output;
}

/**
 * Implements hook_preprocess_HOOK().
 *
 */
function dss_eastasia_process_islandora_solr_facet_pages_results(&$vars) {

  $vars['complex_results'] = array();

  foreach($vars['results'] as $result => $count) {

    switch($vars['solr_field']) {
    case 'eastasia.Date.Artifact.Upper':
    case 'eastasia.Date.Artifact.Lower':
    case 'eastasia.Date.Image.Upper':
    case 'eastasia.Date.Image.Lower':
      
      $filter = $vars['solr_field'] . ':"' . addslashes($result) . 'T00:00:00Z"';
      break;

    default:
      $filter = $vars['solr_field'] . ':"' . addslashes($result) . '"';
    }

    $vars['complex_results'][$result] = array('count' => $count,
					      'filter' => $filter);
  }
}