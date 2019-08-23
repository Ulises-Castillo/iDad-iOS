//
//  IDadModel.swift
//  iDad
//
//  Created by Ulysses Castillo on 8/20/19.
//  Copyright © 2019 uly. All rights reserved.
//

import UIKit

struct IDadViewModel {
    let name: String // Jordan B. Peterson
    var photos: [UIImage] = []
    var videos: [String] = []
    var quotes: [String] = []
    var books: [Book] = []
    var summary: String? = nil // "The 50 Billion dollar man"
    
    // Future Params
    // let fullName: String? // for ex: Nutn = nil ("unplublicized") & Dan Peña = Daniel S. Peña
    // let wikipedia: URL

    var topQuote: String? { return quotes.first }
    var topVideo: String? { return videos.first }
}

struct Book {
    let title: String
    let cover: UIImage
    let url: URL?
}
