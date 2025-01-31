import fetch from 'sync-fetch';

const url = 'https://jsonplaceholder.typicode.com/posts';
const response = fetch(url);

console.log(response.text()); // The response body
