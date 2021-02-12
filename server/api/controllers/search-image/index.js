'use strict';

const SEARCH_API_KEY = process.env.SEARCH_API_KEY || '【APIキー】';
const SEARCH_CSE_ID = process.env.SEARCH_CSE_ID || '【検索エンジンID】';

const HELPER_BASE = process.env.HELPER_BASE || '../../helpers/';
const Response = require(HELPER_BASE + 'response');
const BinResponse = require(HELPER_BASE + 'binresponse');

const sharp = require('sharp')
const fetch = require('node-fetch');
const { google } = require('googleapis');
const customSearch = google.customsearch('v1');

exports.handler = async (event, context, callback) => {
	if( event.path == '/search-image'){
		console.log(event.queryStringParameters);

		var keyword = event.queryStringParameters.keyword || 'ミニオン';
		var num = event.queryStringParameters.num || 10;
		var width = event.queryStringParameters.width || 320;
		var height = event.queryStringParameters.height || 240;

		var link = await search_image(keyword, num);
		console.log(link);

		var buffer = await download_image(link, width, height);

		return new BinResponse('image/jpeg', buffer);
	}
};

async function search_image(keyword, num = 10){
	var index = Math.floor(Math.random() * num);
	const result = await customSearch.cse.list({
		cx: SEARCH_CSE_ID,
		q: keyword,
		auth: SEARCH_API_KEY,
		searchType: 'image',
		safe: 'high',
		num: 1, // max:10
		start: index + 1,
	});

	return result.data.items[0].link;
}

async function download_image(url, width, height){
  const blob = await fetch(url)
  .then(response =>{
    if( !response.ok )
      throw 'status is not 200';
    return response.blob();
  });
	
  const buffer = await blob.arrayBuffer();

	return sharp(new Uint8Array(buffer))
  .resize({ width: width, height: height })
	.toFormat('jpeg')
  .toBuffer();
}
